#include "stdafx.h"

#include <algorithm>
#include <string>
#include <vector>

#include "Inject.h"
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// @Cleanup: @Remark:
// According to Microsoft documentation for BCM_SETSHIELD:
// > An application must be manifested to use comctl32.dll
// > version 6 to gain this functionality.
// If we don't do that, the shield icon doesn't show up, but is there
// a nice way to add that? (i.e. not through a pragma)
#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

struct InjectProcess
{
    InjectProcess(bool injected, Process &&process, std::wstring &&charname)
        : m_Injected(injected), m_Process(std::move(process)), m_Charname(std::move(charname))
    {
    }

    InjectProcess(const InjectProcess &) = delete;
    InjectProcess(InjectProcess &&) = default;

    InjectProcess &operator=(const InjectProcess &) = delete;
    InjectProcess &operator=(InjectProcess &&) = default;

    bool m_Injected;
    Process m_Process;
    std::wstring m_Charname;
};

static bool FindTopMostProcess(std::vector<InjectProcess> &processes, size_t *TopMostIndex)
{
    auto hWndIt = GetTopWindow(nullptr);
    if (hWndIt == nullptr)
    {
        fprintf(stderr, "GetTopWindow failed (%lu)\n", GetLastError());
        return false;
    }

    while (hWndIt != nullptr)
    {
        DWORD WindowPid;
        if (GetWindowThreadProcessId(hWndIt, &WindowPid) == 0)
        {
            fprintf(stderr, "GetWindowThreadProcessId returned 0\n");
            continue;
        }

        for (size_t i = 0; i < processes.size(); ++i)
        {
            if (processes[i].m_Process.GetProcessId() == WindowPid)
            {
                *TopMostIndex = i;
                return true;
            }
        }

        hWndIt = GetWindow(hWndIt, GW_HWNDNEXT);
    }

    return true;
}

static void GetGuildWarsProcesses(std::vector<Process> &processes)
{
    GetProcesses(processes, L"Gw.exe");

    std::vector<Process> buffer;
    GetProcessesFromWindowClass(buffer, L"ArenaNet_Dx_Window_Class");

    for (Process &process : buffer)
    {
        const auto pid = process.GetProcessId();

        auto found = false;
        for (Process &it : processes)
        {
            if (it.GetProcessId() == pid)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            processes.emplace_back(std::move(process));
        }
    }
}

InjectReply InjectWindow::AskInjectProcess(Process *target_process)
{
    std::vector<Process> processes;
    GetGuildWarsProcesses(processes);

    if (processes.empty())
    {
        fprintf(stderr, "Didn't find any potential process to inject HelperBox\n");
        return InjectReply_NoProcess;
    }

    uintptr_t charname_rva;
    ProcessScanner scanner(&processes[0]);
    if (!scanner.FindPatternRva("\x8B\xF8\x6A\x03\x68\x0F\x00\x00\xC0\x8B\xCF\xE8",
                                "xxxxxxxxxxxx",
                                -0x42,
                                &charname_rva))
    {
        return InjectReply_PatternError;
    }

    uintptr_t email_rva;
    if (!scanner.FindPatternRva("\x33\xC0\x5D\xC2\x10\x00\xCC\x68\x80\x00\x00\x00", "xxxxxxxxxxxx", 0xE, &email_rva))
    {
        return InjectReply_PatternError;
    }

    std::vector<InjectProcess> inject_processes;

    for (Process &process : processes)
    {
        ProcessModule module;

        if (!process.GetModule(&module))
        {
            fprintf(stderr, "Couldn't get module for process %u\n", process.GetProcessId());
            continue;
        }

        auto injected = false;
        ProcessModule module2;
        if (process.GetModule(&module2, L"HelperBoxDll.dll"))
        {
            injected = true;
        }

        auto charname_ptr = uint32_t{0};
        if (!process.Read(module.base + charname_rva, &charname_ptr, 4))
        {
            fprintf(stderr,
                    "Can't read the address 0x%08X in process %u\n",
                    module.base + charname_rva,
                    process.GetProcessId());
            continue;
        }
        auto email_ptr = uint32_t{0};
        if (!process.Read(module.base + email_rva, &email_ptr, 4))
        {
            fprintf(stderr,
                    "Can't read the address 0x%08X in process %u\n",
                    module.base + email_rva,
                    process.GetProcessId());
            continue;
        }
        wchar_t charname[128] = {0};
        if (!process.Read(charname_ptr, charname, 20 * sizeof(wchar_t)))
        {
            fprintf(stderr,
                    "Can't read the character name at address 0x%08X in process %u\n",
                    charname_ptr,
                    process.GetProcessId());
            continue;
        }
        if (!charname[0])
        {
            char email[_countof(charname)] = {0};
            if (!process.Read(email_ptr, email, _countof(email) - 1))
            {
                fprintf(stderr,
                        "Can't read the email at address 0x%08X in process %u\n",
                        email_ptr,
                        process.GetProcessId());
                continue;
            }
            for (int i = 0; i < _countof(email) && email[i]; i++)
            {
                charname[i] = email[i];
            }
        }
        if (!charname[0])
        {
            fprintf(stderr, "Character name in process %u is empty\n", process.GetProcessId());
            continue;
        }

        auto charname_len = wcsnlen(charname, _countof(charname));
        std::wstring charname2(charname, charname + charname_len);

        inject_processes.emplace_back(injected, std::move(process), std::wstring(charname2));
    }

    processes.clear();

    if (!inject_processes.size())
    {
        return InjectReply_NoValidProcess;
    }

    if (inject_processes.size() == 1)
    {
        *target_process = std::move(inject_processes[0].m_Process);
        return InjectReply_Inject; // Inject if 1 process found
    }

    // Sort by name
    std::sort(inject_processes.begin(), inject_processes.end(), [](InjectProcess &proc1, InjectProcess &proc2) {
        return proc1.m_Charname < proc2.m_Charname;
    });

    InjectWindow inject;
    inject.Create();

    for (size_t i = 0; i < inject_processes.size(); i++)
    {
        auto process = &inject_processes[i];

        wchar_t buffer[128];
        wcsncpy(buffer, process->m_Charname.c_str(), _countof(buffer));
        if (process->m_Injected)
            wcsncat(buffer, L" (injected)", _countof(buffer));

        SendMessageW(inject.m_hCharacters, CB_ADDSTRING, 0, (LPARAM)buffer);
    }

    size_t TopMostIdx;
    if (FindTopMostProcess(inject_processes, &TopMostIdx))
        SendMessageW(inject.m_hCharacters, CB_SETCURSEL, static_cast<WPARAM>(TopMostIdx), 0);
    else
        SendMessageW(inject.m_hCharacters, CB_SETCURSEL, 0, 0);

    inject.WaitMessages();

    size_t index;
    if (!inject.GetSelected(&index))
    {
        fprintf(stderr, "No index selected\n");
        return InjectReply_Cancel;
    }

    *target_process = std::move(inject_processes[index].m_Process);
    return InjectReply_Inject;
}

InjectWindow::InjectWindow() : m_hCharacters(nullptr), m_hLaunchButton(nullptr), m_Selected(-1)
{
}

InjectWindow::~InjectWindow()
{
}

bool InjectWindow::Create()
{
    SetWindowName(L"HelperBox - Launch");
    SetWindowDimension(305, 135);
    return Window::Create();
}

bool InjectWindow::GetSelected(size_t *index)
{
    if (m_Selected >= 0)
    {
        *index = static_cast<size_t>(m_Selected);
        return true;
    }
    else
    {
        return false;
    }
}

LRESULT InjectWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        OnCreate(hWnd, uMsg, wParam, lParam);
        break;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        SignalStop();
        break;

    case WM_COMMAND:
        OnCommand(reinterpret_cast<HWND>(lParam), LOWORD(wParam), HIWORD(wParam));
        break;

    case WM_KEYUP:
        if (wParam == VK_ESCAPE)
        {
            DestroyWindow(hWnd);
        }
        else if (wParam == VK_RETURN)
        {
            m_Selected = SendMessageW(m_hCharacters, CB_GETCURSEL, 0, 0);
            DestroyWindow(hWnd);
        }
        break;
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void InjectWindow::OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    CreateWindowW(WC_BUTTONW,
                  L"Select Character",
                  WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                  10,
                  5,
                  270,
                  55,
                  hWnd,
                  nullptr,
                  m_hInstance,
                  nullptr);

    m_hCharacters = CreateWindowW(WC_COMBOBOXW,
                                  L"",
                                  WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_TABSTOP | CBS_DROPDOWNLIST,
                                  20,  // x
                                  25,  // y
                                  155, // width
                                  25,  // height
                                  hWnd,
                                  nullptr,
                                  m_hInstance,
                                  nullptr);

    m_hLaunchButton = CreateWindowW(WC_BUTTONW,
                                    L"Launch",
                                    WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON,
                                    180, // x
                                    24,  // y
                                    90,  // width
                                    25,  // height
                                    hWnd,
                                    nullptr,
                                    m_hInstance,
                                    nullptr);
}

void InjectWindow::OnCommand(HWND hWnd, LONG ControlId, LONG NotificationCode)
{
    UNREFERENCED_PARAMETER(NotificationCode);

    if ((hWnd == m_hLaunchButton) && (ControlId == STN_CLICKED))
    {
        m_Selected = SendMessageW(m_hCharacters, CB_GETCURSEL, 0, 0);
        DestroyWindow(m_hWnd);
    }
}

static LPVOID GetLoadLibrary()
{
    auto Kernel32 = GetModuleHandleW(L"Kernel32.dll");
    if (Kernel32 == nullptr)
    {
        fprintf(stderr, "GetModuleHandleW failed (%lu)\n", GetLastError());
        return nullptr;
    }

    auto pLoadLibraryW = GetProcAddress(Kernel32, "LoadLibraryW");
    if (pLoadLibraryW == nullptr)
    {
        fprintf(stderr, "GetProcAddress failed (%lu)\n", GetLastError());
        return nullptr;
    }

    return pLoadLibraryW;
}

bool InjectRemoteThread(Process *process, LPCWSTR ImagePath, LPDWORD lpExitCode)
{
    *lpExitCode = 0;

    auto ProcessHandle = process->GetHandle();
    if (ProcessHandle == nullptr)
    {
        fprintf(stderr, "Can't inject a dll in a process which is not open\n");
        return FALSE;
    }

    auto pLoadLibraryW = GetLoadLibrary();
    if (pLoadLibraryW == nullptr)
        return FALSE;

    auto ImagePathLength = wcslen(ImagePath);
    auto ImagePathSize = (ImagePathLength * 2) + 2;

    LPVOID ImagePathAddress =
        VirtualAllocEx(ProcessHandle, nullptr, ImagePathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (ImagePathAddress == nullptr)
    {
        fprintf(stderr, "VirtualAllocEx failed (%lu)\n", GetLastError());
        return FALSE;
    }

    SIZE_T BytesWritten;
    auto Success = WriteProcessMemory(ProcessHandle, ImagePathAddress, ImagePath, ImagePathSize, &BytesWritten);

    if (!Success || (ImagePathSize != BytesWritten))
    {
        fprintf(stderr, "WriteProcessMemory failed (%lu)\n", GetLastError());
        VirtualFreeEx(ProcessHandle, ImagePathAddress, 0, MEM_RELEASE);
        return FALSE;
    }

    DWORD ThreadId;
    auto hThread = CreateRemoteThreadEx(ProcessHandle,
                                        nullptr,
                                        0,
                                        reinterpret_cast<LPTHREAD_START_ROUTINE>(pLoadLibraryW),
                                        ImagePathAddress,
                                        0,
                                        nullptr,
                                        &ThreadId);

    if (hThread == nullptr)
    {
        fprintf(stderr, "CreateRemoteThreadEx failed (%lu)\n", GetLastError());
        return FALSE;
    }

    auto Reason = WaitForSingleObject(hThread, INFINITE);
    if (Reason != WAIT_OBJECT_0)
    {
        fprintf(stderr, "WaitForSingleObject failed {reason: %lu, error: %lu}\n", Reason, GetLastError());
        CloseHandle(hThread);
        return FALSE;
    }

    VirtualFreeEx(ProcessHandle, ImagePathAddress, 0, MEM_RELEASE);

    DWORD ExitCode;
    Success = GetExitCodeThread(hThread, &ExitCode);
    CloseHandle(hThread);

    if (Success == FALSE)
    {
        fprintf(stderr, "GetExitCodeThread failed (%lu)\n", GetLastError());
        return FALSE;
    }

    *lpExitCode = ExitCode;
    return TRUE;
}

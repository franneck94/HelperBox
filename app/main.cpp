#include "stdafx.h"

#include "Inject.h"
#include "Path.h"
#include "Process.h"
#include "Settings.h"

static void ShowError(const wchar_t *message)
{
    MessageBoxW(0, message, L"HelperBox - Error", 0);
}

static bool InjectInstalledDllInProcess(Process *process)
{
    ProcessModule module;
    if (process->GetModule(&module, L"HelperBoxDll.dll"))
    {
        MessageBoxW(0, L"HelperBox is already running in this process", L"HelperBox", 0);
        return true;
    }

    wchar_t dllpath[MAX_PATH];
    PathGetProgramDirectory(dllpath, MAX_PATH);

    PathCompose(dllpath, MAX_PATH, dllpath, L"HelperBoxDll.dll");

    DWORD ExitCode;
    if (!InjectRemoteThread(process, dllpath, &ExitCode))
    {
        fprintf(stderr, "InjectRemoteThread failed (ExitCode: %lu)\n", ExitCode);
        return false;
    }

    return true;
}

static bool SetProcessForeground(Process *process)
{
    HWND hWndIt = GetTopWindow(nullptr);
    if (hWndIt == nullptr)
    {
        fprintf(stderr, "GetTopWindow failed (%lu)\n", GetLastError());
        return false;
    }

    DWORD ProcessId = process->GetProcessId();

    while (hWndIt != nullptr)
    {
        DWORD WindowPid;
        if (GetWindowThreadProcessId(hWndIt, &WindowPid) == 0)
        {
            fprintf(stderr, "GetWindowThreadProcessId returned 0\n");
            continue;
        }

        if (WindowPid == ProcessId)
        {
            SetForegroundWindow(hWndIt);
            return true;
        }

        hWndIt = GetWindow(hWndIt, GW_HWNDNEXT);
    }

    return false;
}

#ifdef _DEBUG
int main()
#else
INT __stdcall WinMain(HINSTANCE, HINSTANCE, PSTR, INT)
#endif
{
    ParseRegSettings();
    ParseCommandLine();

    Process proc;

    if (settings.pid)
    {
        if (!proc.Open(settings.pid))
        {
            fprintf(stderr, "Couldn't open process %d\n", settings.pid);
            return 1;
        }

        if (!InjectInstalledDllInProcess(&proc))
        {
            fprintf(stderr, "InjectInstalledDllInProcess failed\n");
            return 1;
        }

        SetProcessForeground(&proc);

        return 0;
    }

    InjectWindow::AskInjectProcess(&proc);

    if (!InjectInstalledDllInProcess(&proc))
    {
        ShowError(L"Couldn't find any appropriate target to start HelperBox");
        fprintf(stderr, "InjectInstalledDllInProcess failed\n");
        return 1;
    }

    SetProcessForeground(&proc);

    return 0;
}

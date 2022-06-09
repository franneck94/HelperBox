#include <windows.h>

#include <psapi.h>
#include <stdio.h>
#include <tchar.h>
#include <tlhelp32.h>

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

void GetAllProcessIdsByName(const std::string targetProcessName, std::vector<DWORD> &pids)
{
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof entry;

    if (!Process32First(snap, &entry))
        return;

    do
    {
        if (std::string(entry.szExeFile) == targetProcessName)
        {
            pids.emplace_back(entry.th32ProcessID);
        }
    } while (Process32Next(snap, &entry));
}

bool ReadWideString(HANDLE hProc, std::uintptr_t addr, std::wstring &out)
{
    out.resize(100);
    SIZE_T numRead = 0;
    if (!ReadProcessMemory(hProc, reinterpret_cast<LPVOID>(addr), &out[0], 100 * sizeof(wchar_t), &numRead))
    {
        out.clear();
        return false;
    }
    out.resize(numRead / sizeof(wchar_t));
    return true;
}

bool tryParse(std::string &input, int &output)
{
    try
    {
        output = std::stoi(input);
    }
    catch (std::invalid_argument)
    {
        return false;
    }
    return true;
}

void CheckForInjectedDll(const std::vector<DWORD> &pids, std::vector<bool> &is_injected)
{
    for (const auto pid : pids)
    {
        HMODULE hMods[1024];
        HANDLE hProcess;
        DWORD cbNeeded;
        unsigned int i;

        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (NULL == hProcess)
            return;

        if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
        {
            for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
            {
                wchar_t szModName[MAX_PATH];
                if (GetModuleBaseNameW(hProcess, hMods[i], szModName, _countof(szModName)))
                {
                    printf("%ls\n", szModName);
                    if (szModName == L"HelperBox.dll")
                    {
                        is_injected[0] = true;
                    }
                }
            }
        }
    }
}

int main()
{
    std::vector<DWORD> pids;
    GetAllProcessIdsByName("Gw.exe", pids);

    std::vector<bool> is_injected(pids.size(), false);
    CheckForInjectedDll(pids, is_injected);

    for (int i = 0; i < pids.size(); ++i)
    {
        std::cout << i << " - GW Process ID: " << pids[i] << " is injected: " << std::boolalpha << is_injected[i]
                  << '\n';
    }

    DWORD pid = 0;
    std::cout << "Select GW Process: ";

    std::string input;
    int idx;
    getline(std::cin, input);
    while (!tryParse(input, idx))
    {
        std::cout << "Bad entry. Enter a NUMBER: ";
        getline(std::cin, input);
    }
    pid = pids[idx];

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    uint64_t size = 100;
    char *message = new char[size]{};
    const auto num_bytes = size * sizeof(char);

    std::uintptr_t dwAddr = 0x010892F0;

    std::wstring w(100, L'\0');
    ReadWideString(hProcess, dwAddr, w);
    std::string s(w.begin(), w.end());
    std::cout << s << std::endl;

    return 0;
}

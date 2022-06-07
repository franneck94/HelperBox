#include <windows.h>

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

int main()
{
    std::vector<DWORD> pids;
    GetAllProcessIdsByName("Gw.exe", pids);

    for (int i = 0; i < pids.size(); ++i)
    {
        std::cout << i << " - GW Process ID: " << pids[i] << '\n';
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

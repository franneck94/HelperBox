#include <windows.h>

#include <tlhelp32.h>

#include <iostream>
#include <string>

DWORD FindProcessId(const std::string &processName)
{
    PROCESSENTRY32 processInfo;
    processInfo.dwSize = sizeof(processInfo);

    HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (processesSnapshot == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    Process32First(processesSnapshot, &processInfo);
    if (!processName.compare(processInfo.szExeFile))
    {
        CloseHandle(processesSnapshot);
        return processInfo.th32ProcessID;
    }

    while (Process32Next(processesSnapshot, &processInfo))
    {
        if (!processName.compare(processInfo.szExeFile))
        {
            CloseHandle(processesSnapshot);
            return processInfo.th32ProcessID;
        }
    }

    CloseHandle(processesSnapshot);
    return 0;
}

int main()
{
    const auto ProcessId = FindProcessId("Gw.exe");
    std::cout << "ProcessId: " << ProcessId << std::endl;
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);

    SIZE_T NumberOfBytesRead;
    constexpr auto size = 4U;
    char Buffer[size] = {0};
    const auto dwAddr = 0x016C5411;
    BOOL success = ReadProcessMemory(hProcess, &dwAddr, &Buffer, size * sizeof(char), &NumberOfBytesRead);

    if (!success)
        return 1;

    std::cout << Buffer << std::endl;

    return 0;
}

#include "stdafx.h"

#include "Registry.h"
#include "Settings.h"

Settings settings;

void PrintUsage(bool terminate)
{
    fprintf(stderr,
            "Usage: [options]\n\n"
            "    /pid <process id>\n");

    if (terminate)
        exit(0);
}

void ParseRegSettings()
{
    HKEY SettingsKey;
    if (!OpenSettingsKey(&SettingsKey))
    {
        fprintf(stderr, "OpenUninstallKey failed\n");
        return;
    }

    RegCloseKey(SettingsKey);
}

void ParseCommandLine()
{
    int argc;
    LPWSTR CmdLine = GetCommandLineW();
    LPWSTR *argv = CommandLineToArgvW(CmdLine, &argc);
    if (argv == nullptr)
    {
        fprintf(stderr, "CommandLineToArgvW failed (%lu)\n", GetLastError());
        return;
    }

    for (int i = 1; i < argc; ++i)
    {
        wchar_t *arg = argv[i];

        if (wcscmp(arg, L"/pid") == 0)
        {
            if (++i == argc)
            {
                fprintf(stderr, "'/pid' must be followed by a process id\n");
                PrintUsage(true);
            }

            int pid = _wtoi(argv[i]);
            if (pid < 0)
            {
                fprintf(stderr, "Process id must be a positive integer");
                exit(0);
            }
            settings.pid = static_cast<uint32_t>(pid);
        }
    }
}

bool CreateProcessInt(const wchar_t *path, const wchar_t *args, const wchar_t *workdir, bool)
{
    wchar_t command_line[1024] = L"";
    size_t n_path = wcslen(path);
    size_t n_args = wcslen(args);
    if ((n_path + n_args + 2) >= _countof(command_line))
        return false;

    wcscat_s(command_line, path);
    wcscat_s(command_line, L" ");
    wcscat_s(command_line, args);

    SHELLEXECUTEINFOW ExecInfo = {0};
    ExecInfo.cbSize = sizeof(ExecInfo);
    ExecInfo.fMask = SEE_MASK_NOASYNC;
    ExecInfo.lpFile = path;
    ExecInfo.lpParameters = args;
    ExecInfo.lpDirectory = workdir;
    ExecInfo.nShow = SW_SHOWNORMAL;

    if (!ShellExecuteExW(&ExecInfo))
    {
        fprintf(stderr, "ShellExecuteExA failed: %lu\n", GetLastError());
        return false;
    }

    return true;
}

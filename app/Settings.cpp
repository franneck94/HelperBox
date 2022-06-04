#include "stdafx.h"

#include "Settings.h"

Settings settings;

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

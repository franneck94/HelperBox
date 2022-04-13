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
    if (settings.localdll)
    {
        PathGetProgramDirectory(dllpath, MAX_PATH);
    }

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
            // @Cleanup:
            // Not clear whether this is the return value hold an error, so we just log.
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
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
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

    InjectReply reply = InjectWindow::AskInjectProcess(&proc);

    if (reply == InjectReply_Cancel)
    {
        fprintf(stderr, "InjectReply_Cancel\n");
        return 0;
    }

    if (reply == InjectReply_PatternError)
    {
        MessageBoxW(0,
                    L"Couldn't find character name RVA.\n"
                    L"You need to update the launcher or contact the developpers.",
                    L"HelperBox - Error",
                    MB_OK | MB_ICONERROR);
        return 1;
    }

    if (reply == InjectReply_NoValidProcess)
    {

        ShowError(L"Failed to inject HelperBox into Guild Wars\n");
        fprintf(stderr, "InjectWindow::AskInjectName failed\n");
        return 0;
    }
    if (reply == InjectReply_NoProcess)
    {
        int iRet = MessageBoxW(0,
                               L"Couldn't find any valid process to start HelperBoxpp.\n"
                               L"Ensure Guild Wars is running before trying to run HelperBox.\n",
                               L"HelperBox - Error",
                               MB_RETRYCANCEL);
        if (iRet == IDCANCEL)
        {
            fprintf(stderr, "User doesn't want to retry\n");
            return 1;
        }
    }

    if (!InjectInstalledDllInProcess(&proc))
    {
        ShowError(L"Couldn't find any appropriate target to start HelperBox");
        fprintf(stderr, "InjectInstalledDllInProcess failed\n");
        return 1;
    }

    SetProcessForeground(&proc);

    return 0;
}

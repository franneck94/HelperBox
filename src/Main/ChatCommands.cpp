#include "stdafx.h"

#include <GWCA/Managers/CameraMgr.h>
#include <GWCA/Managers/ChatMgr.h>

#include <Base/HelperBoxWindow.h>
#include <Base/MainWindow.h>
#include <HelperBox.h>
#include <Logger.h>

#include "ChatCommands.h"

void ChatCommands::DrawSettingInternal()
{
}

void ChatCommands::LoadSettings(CSimpleIni *)
{
}

void ChatCommands::SaveSettings(CSimpleIni *)
{
}

void ChatCommands::Initialize()
{
    HelperBoxModule::Initialize();
    GW::Chat::CreateCommand(L"hb", ChatCommands::CmdHB);
}

void ChatCommands::Update(float)
{
}

void ChatCommands::CmdHB(const wchar_t *, int argc, LPWSTR *argv)
{
    if (argc == 2)
    {
        const std::wstring arg = argv[1];
        if (arg == L"close" || arg == L"quit" || arg == L"exit")
        {
            HelperBox::Instance().StartSelfDestruct();
            return;
        }
    }

    if (argc < 3)
    {
        const std::wstring arg = argv[1];
        if (arg == L"hide")
        {
            MainWindow::Instance().visible = false;
            return;
        }
        if (arg == L"show")
        {
            MainWindow::Instance().visible = true;
            return;
        }
        if (arg == L"save")
        {
            HelperBox::Instance().SaveSettings();
            return;
        }
        if (arg == L"load")
        {
            HelperBox::Instance().OpenSettingsFile();
            HelperBox::Instance().LoadModuleSettings();
            return;
        }
    }

    Log::Error("Unknown command!");
}

#include "stdafx.h"

#include <GWCA/Managers/CameraMgr.h>
#include <GWCA/Managers/ChatMgr.h>

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

    Log::Error("Unknown command!");
}

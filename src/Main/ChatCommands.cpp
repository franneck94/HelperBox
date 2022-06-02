#include "stdafx.h"

#include <GWCA/Managers/CameraMgr.h>
#include <GWCA/Managers/ChatMgr.h>

#include <HelperBox.h>
#include <Logger.h>

#include "ChatCommands.h"

void ChatCommands::DrawSettingInternal()
{
}

void ChatCommands::LoadSettings(CSimpleIni *ini)
{
    UNREFERENCED_PARAMETER(ini);
}

void ChatCommands::SaveSettings(CSimpleIni *ini)
{
    UNREFERENCED_PARAMETER(ini);
}

void ChatCommands::Initialize()
{
    HelperBoxModule::Initialize();
    GW::Chat::CreateCommand(L"hb", ChatCommands::CmdHB);
}

void ChatCommands::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);
}

void ChatCommands::CmdHB(const wchar_t *message, int argc, LPWSTR *argv)
{
    UNREFERENCED_PARAMETER(message);

    if (argc == 2)
    {
        const std::wstring arg = argv[1];
        if (arg == L"close" || arg == L"quit" || arg == L"exit")
            HelperBox::Instance().StartSelfDestruct();
        else
            Log::Error("Unknown command!");
    }
    else
        Log::Error("Unknown command!");
}

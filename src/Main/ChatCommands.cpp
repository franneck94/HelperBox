#include "stdafx.h"

#include <GWCA/Managers/CameraMgr.h>
#include <GWCA/Managers/ChatMgr.h>

#include <HelperBox.h>

#include "ChatCommands.h"

void ChatCommands::DrawHelp()
{
}

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
    GW::Chat::CreateCommand(L"hb", ChatCommands::CmdTB);
}

void ChatCommands::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);
}

bool ChatCommands::WndProc(UINT, WPARAM, LPARAM)
{
    return false;
};

void ChatCommands::CmdTB(const wchar_t *message, int argc, LPWSTR *argv)
{
    UNREFERENCED_PARAMETER(message);

    std::cout << "Called\n";

    if (argc == 2)
    {
        std::cout << "Called 2\n";

        const std::wstring arg = argv[1];
        if (arg == L"close" || arg == L"quit" || arg == L"exit")
        {
            HelperBox::Instance().StartSelfDestruct();
        }
        return;
    }
}

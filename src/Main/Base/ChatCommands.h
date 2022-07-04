#pragma once

#include <GWCA/Constants/Constants.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/Managers/UIMgr.h>
#include <GWCA/Utilities/Hook.h>

#include <Base/HelperBoxModule.h>
#include <Base/HelperBoxUIElement.h>

class ChatCommands : public HelperBoxModule
{
    ChatCommands(){};
    ~ChatCommands(){};

public:
    static ChatCommands &Instance()
    {
        static ChatCommands instance;
        return instance;
    }

    const char *Name() const override
    {
        return "Chat Commands";
    }
    const char *SettingsName() const override
    {
        return "Chat Settings";
    }

    void Initialize() override;

    static void CmdHB(const wchar_t *message, int argc, LPWSTR *argv);
};

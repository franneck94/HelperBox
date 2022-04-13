#pragma once

#include <GWCA/Constants/Constants.h>
#include <GWCA/Utilities/Hook.h>

#include <GWCA/GameContainers/GamePos.h>

#include <GWCA/Managers/UIMgr.h>

#include <HelperBoxModule.h>
#include <HelperBoxUIElement.h>

class ChatCommands : public HelperBoxModule
{
    const float DEFAULT_CAM_SPEED = 1000.f;                      // 600 units per sec
    const float ROTATION_SPEED = static_cast<float>(M_PI) / 3.f; // 6 seconds for full rotation

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
        return "Game Settings";
    }

    void Initialize() override;
    void LoadSettings(CSimpleIni *ini) override;
    void SaveSettings(CSimpleIni *ini) override;
    void DrawSettingInternal() override;

    void DrawHelp() override;
    bool WndProc(UINT Message, WPARAM wParam, LPARAM lParam);
    void Update(float delta) override;

    static void CmdTB(const wchar_t *message, int argc, LPWSTR *argv);
};

#pragma once

#include <GWCA/Constants/Constants.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/Managers/UIMgr.h>
#include <GWCA/Utilities/Hook.h>

#include <Base/HelperBoxModule.h>
#include <Base/HelperBoxUIElement.h>
#include <DataLivings.h>
#include <Timer.h>

class ChatCommands : public HelperBoxModule
{
public:
    ChatCommands(){};
    ~ChatCommands(){};

private:
    struct SkillToUse
    {
        uint32_t slot = 0; // 1-8 range
        float skill_usage_delay = 0.f;
        clock_t skill_timer = clock();
        void Update();
    } skill_to_use;
    const AgentLivingData *livings_data = nullptr;

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
    void Update(float delta, const AgentLivingData &livings_data) override;

    static void CmdHB(const wchar_t *message, int argc, LPWSTR *argv);
    static void CmdDhuumUseSkill(const wchar_t *, int argc, LPWSTR *argv);
};

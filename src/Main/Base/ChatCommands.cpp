#include <string>

#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/ChatMgr.h>

#include <Base/HelperBox.h>
#include <Base/HelperBoxWindow.h>
#include <Base/MainWindow.h>
#include <HelperMaps.h>
#include <HelperUw.h>
#include <Logger.h>

#include "ChatCommands.h"

void ChatCommands::Initialize()
{
    HelperBoxModule::Initialize();
    GW::Chat::CreateCommand(L"hb", ChatCommands::CmdHB);
    GW::Chat::CreateCommand(L"dhuum", ChatCommands::CmdDhuumUseSkill);
}

void ChatCommands::Update(float, const AgentLivingData &)
{
    if (!IsMapReady())
        return;

    skill_to_use.Update();
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
}

void ChatCommands::SkillToUse::Update()
{
    if (!slot)
        return;
    if (IsMapReady())
    {
        slot = 0;
        return;
    }
    if ((clock() - skill_timer) / 1000.0f < skill_usage_delay)
        return;
    const auto skillbar = GW::SkillbarMgr::GetPlayerSkillbar();
    if (!skillbar || !skillbar->IsValid())
    {
        slot = 0;
        return;
    }
    const auto me = GW::Agents::GetPlayer();
    if (!me)
        return;
    const auto me_living = me->GetAsAgentLiving();
    if (!me_living)
        return;

    uint32_t lslot = slot - 1;
    const auto &skill = skillbar->skills[lslot];
    const GW::Skill &skilldata = *GW::SkillbarMgr::GetSkillConstantData(skill.skill_id);
    if (skill.GetRecharge() == 0 && me_living->energy >= skilldata.energy_cost &&
        ((skilldata.adrenaline == 0) || (skilldata.adrenaline > 0 && skill.adrenaline_a >= skilldata.adrenaline)))
    {
        GW::SkillbarMgr::UseSkill(lslot, GW::Agents::GetTargetId());
        skill_usage_delay = skilldata.activation + skilldata.aftercast;
        skill_timer = clock();
    }
}

void ChatCommands::CmdDhuumUseSkill(const wchar_t *, int argc, LPWSTR *argv)
{
    if (!IsMapReady())
        return;

    auto &skill_to_use = Instance().skill_to_use;
    skill_to_use.slot = 0;

    if (argc < 2)
        return;
    const auto arg1 = std::wstring{argv[1]};
    if (arg1 == L"stop" || arg1 == L"off" || arg1 == L"0")
        return;

    auto dhuum_id = uint32_t{0};
    auto dhuum_hp = float{1.0F};
    const auto is_in_dhuum_fight = IsInDhuumFight(&dhuum_id, &dhuum_hp);
    if (!is_in_dhuum_fight)
        return;

    uint32_t num = _wtoi(argv[1]);
    if (!num || num < 1 || num > 8)
    {
        Log::Error("Invalid argument '%ls', please use an integer value of 1 to 8", argv[1]);
        return;
    }
    skill_to_use.slot = num;
    skill_to_use.skill_usage_delay = 0.0F;
}

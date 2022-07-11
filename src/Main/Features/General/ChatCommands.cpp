#include <cstdlib>
#include <string>

#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>

#include <Base/HelperBox.h>
#include <Base/HelperBoxWindow.h>
#include <Base/MainWindow.h>
#include <HelperAgents.h>
#include <HelperMaps.h>
#include <HelperUw.h>
#include <Logger.h>

#include "ChatCommands.h"

void ChatCommands::Initialize()
{
    HelperBoxModule::Initialize();
    GW::Chat::CreateCommand(L"hb", ChatCommands::CmdHB);
    GW::Chat::CreateCommand(L"use", ChatCommands::CmdUseSkill);
    GW::Chat::CreateCommand(L"dhuum", ChatCommands::CmdDhuumUseSkill);
}

void ChatCommands::Update(float, const AgentLivingData &)
{
    if (!IsMapReady())
    {
        useskill.slot = 0;
        dhuum_useskill.slot = 0;
        return;
    }

    useskill.Update();
    dhuum_useskill.Update();
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

void ChatCommands::BaseUseSkill::CastSelectedSkill(const uint32_t current_energy,
                                                   const GW::Skillbar *skillbar,
                                                   const uint32_t target_id)
{
    const auto lslot = slot - 1;
    const auto &skill = skillbar->skills[lslot];
    const auto skilldata = GW::SkillbarMgr::GetSkillConstantData(skill.skill_id);
    if (!skilldata)
        return;

    const auto enough_energy = current_energy > skilldata->energy_cost;
    const auto enough_adrenaline =
        (skilldata->adrenaline == 0) || (skilldata->adrenaline > 0 && skill.adrenaline_a >= skilldata->adrenaline);
    if (skill.GetRecharge() == 0 && enough_energy && enough_adrenaline)
    {
        if (target_id)
            GW::SkillbarMgr::UseSkill(lslot, target_id);
        else
            GW::SkillbarMgr::UseSkill(lslot, GW::Agents::GetTargetId());
        skill_usage_delay = skilldata->activation + skilldata->aftercast;
        skill_timer = clock();
    }
}

void ChatCommands::UseSkill::Update()
{
    if (slot == 0)
        return;
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

    if (!slot)
        return;

    const auto current_energy = static_cast<uint32_t>(me_living->energy * me_living->max_energy);
    CastSelectedSkill(current_energy, skillbar);
}

void ChatCommands::DhuumUseSkill::Update()
{
    if (slot == 0)
        return;
    if ((clock() - skill_timer) / 1000.0f < skill_usage_delay)
        return;
    const auto skillbar = GW::SkillbarMgr::GetPlayerSkillbar();
    if (!skillbar || !skillbar->IsValid())
    {
        slot = 0;
        return;
    }

    const auto me_living = GetPlayerAsLiving();
    if (!me_living)
        return;

    const auto progress_perc = GetProgressValue();
    auto target_id = uint32_t{0};
    if (progress_perc < 1.0F)
    {
        slot = 1;
    }
    else // Rest done
    {
        const auto dhuum_agent = GetDhuumAgent();
        if (!dhuum_agent)
        {
            slot = 0;
            return;
        }
        const auto dhuum_living = dhuum_agent->GetAsAgentLiving();
        const auto target = GetTargetAsLiving();
        if (!dhuum_living || dhuum_living->allegiance != GW::Constants::Allegiance::Enemy || !target ||
            target->player_number != static_cast<uint16_t>(GW::Constants::ModelID::UW::Dhuum))
        {
            slot = 0;
            return;
        }

        slot = 5;
        target_id = target->agent_id;

        if (!me_living->GetIsAttacking())
            AttackAgent(target);
    }

    if (!slot)
        return;

    const auto current_energy = static_cast<uint32_t>((me_living->energy * me_living->max_energy));
    CastSelectedSkill(current_energy, skillbar, target_id);
}

void ChatCommands::CmdDhuumUseSkill(const wchar_t *, int argc, LPWSTR *argv)
{
    if (!IsMapReady() && IsExplorable())
        return;

    auto &dhuum_useskill = Instance().dhuum_useskill;

    dhuum_useskill.skill_usage_delay = 0.0F;
    dhuum_useskill.slot = 0;

    if (argc < 2)
        return;
    const auto arg1 = std::wstring{argv[1]};
    if (arg1 != L"start" || arg1 == L"end" || arg1 == L"stop")
        return;

    dhuum_useskill.slot = static_cast<uint32_t>(-1);
}

void ChatCommands::CmdUseSkill(const wchar_t *, int argc, LPWSTR *argv)
{
    if (!IsMapReady() && IsExplorable())
        return;

    auto &useskill = Instance().useskill;

    useskill.skill_usage_delay = 0.0F;
    useskill.slot = 0;

    if (argc < 2)
        return;

    const auto arg1 = std::wstring{argv[1]};
    const auto arg1_int = _wtoi(arg1.data());
    if (arg1_int == 0 || arg1_int < 1 || arg1_int > 8)
        return;

    useskill.slot = arg1_int;
}

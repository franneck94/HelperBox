#include <string>

#include <GWCA/Context/CharContext.h>
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

void ChatCommands::Update(float, const AgentLivingData &_livings_data)
{
    if (!IsMapReady())
    {
        skill_to_use.slot = 0;
        return;
    }

    livings_data = &_livings_data;
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

    const auto enough_energy = (me_living->energy * me_living->max_energy) > skilldata.energy_cost;
    const auto enough_adrenaline =
        (skilldata.adrenaline == 0) || (skilldata.adrenaline > 0 && skill.adrenaline_a >= skilldata.adrenaline);
    if (skill.GetRecharge() == 0 && enough_energy && enough_adrenaline)
    {
        GW::SkillbarMgr::UseSkill(lslot);
        skill_usage_delay = skilldata.activation + skilldata.aftercast;
        skill_timer = clock();
    }
}

static float GetProgressValue()
{
    const auto c = GW::CharContext::instance();

    if (!c || !c->progress_bar)
        return 0.0F;

    return c->progress_bar->progress;
}

void ChatCommands::CmdDhuumUseSkill(const wchar_t *, int argc, LPWSTR *argv)
{
    if (!IsMapReady())
        return;

    auto &skill_to_use = Instance().skill_to_use;
    const auto livings_data = Instance().livings_data;

    skill_to_use.skill_usage_delay = 0.0F;
    skill_to_use.slot = 0;

    if (argc < 2)
        return;
    const auto arg1 = std::wstring{argv[1]};
    if (arg1 != L"start")
        return;

    const auto dhuum_agent = GetDhuumAgent();
    if (!dhuum_agent)
        return;

    skill_to_use.slot = 1;

    const auto progress_perc = GetProgressValue();
    if (progress_perc > 0.99F)
        skill_to_use.slot = 5;

    if (!livings_data)
        return;
    const auto dhuum_fight_done = DhuumFightDone(livings_data->npcs) || DhuumFightDone(livings_data->neutrals);
    if (dhuum_fight_done)
        skill_to_use.slot = 0;
}

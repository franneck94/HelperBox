#include <array>
#include <cstdint>
#include <future>
#include <memory>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/ItemIDs.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/GameContainers/Array.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Player.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/ItemMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/MerchantMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Managers/PlayerMgr.h>
#include <GWCA/Managers/SkillbarMgr.h>
#include <GWCA/Managers/TradeMgr.h>

#include <HelperBox.h>
#include <Logger.h>
#include <Timer.h>

#include <fmt/format.h>

#include <Actions.h>
#include <Helper.h>
#include <Utils.h>

#include "ChestRunnerWindow.h"

namespace
{
static bool issue_resign = false;
} // namespace

void ChestRunnerWindow::AddChestActions()
{
    actions.push_back(std::make_shared<ChangeTargetAction>("targetChest", TargetType::Gadget));
    actions.push_back(std::make_shared<OpenChestAction>("openChest"));
    actions.push_back(std::make_shared<ChangeTargetAction>("targetItem", TargetType::Item));
    actions.push_back(std::make_shared<GotoTargetAction>("gotoItem"));
}

void ChestRunnerWindow::Initialize()
{
    HelperBoxWindow::Initialize();

    static bool added = false;

    if (!added)
    {
        Log::Log(fmt::format("Loaded Helper").data());

        constexpr auto target_map = GW::Constants::MapID::Tanglewood_Copse_outpost;
        constexpr auto target_region = GW::Constants::MapRegion::European;
        constexpr auto target_language = GW::Constants::MapLanguage::Polish;

        const auto positions = std::vector<GW::GamePos>{GW::GamePos{-17884.62F, -9500.90F, 0}, //
                                                        GW::GamePos{-13278.06F, -4923.35F, 0}, //
                                                        GW::GamePos{-13339.81F, -2421.60F, 0}, //
                                                        GW::GamePos{-13429.45F, -1302.20F, 0}, //
                                                        GW::GamePos{-14317.77F, -594.48F, 0},  //
                                                        GW::GamePos{-14978.20F, 677.33F, 0},   //
                                                        GW::GamePos{-15323.66F, 3659.63F, 0},  //
                                                        GW::GamePos{-12232.33F, 3060.701F, 0}, //
                                                        GW::GamePos{-8627.49F, 2509.15F, 0},   //
                                                        GW::GamePos{-6305.65F, 5060.01F, 0},   //
                                                        GW::GamePos{-2734.25F, 6202.46F, 0},   //
                                                        GW::GamePos{892.99F, 5752.73F, 0}};    //
        const auto num_actions = positions.size();

        actions.push_back(std::make_shared<TravelAction>("travel1", target_map, target_region, target_language));
        actions.push_back(std::make_shared<LoadSkillTemplateAction>("loadBar", skillbar.code));
        actions.push_back(std::make_shared<WalkAction>("pos0", positions[0]));
        actions.push_back(std::make_shared<WalkAction>("pos1", positions[1]));
        actions.push_back(std::make_shared<WalkAction>("pos2", positions[2]));
        actions.push_back(std::make_shared<WalkAction>("pos3", positions[3]));
        for (uint32_t ChestSpotIdx = 4; ChestSpotIdx < num_actions; ChestSpotIdx++)
        {
            const auto name = fmt::format("pos{}", ChestSpotIdx);
            actions.push_back(std::make_shared<WalkAction>(name, positions[ChestSpotIdx]));
            AddChestActions();
        }

        for (const auto &action : actions)
        {
            types.push_back(action->type);
        }

        added = true;
    }
}

void ChestRunnerWindow::LoadSettings(CSimpleIni *ini)
{
    HelperBoxWindow::LoadSettings(ini);
}

void ChestRunnerWindow::SaveSettings(CSimpleIni *ini)
{
    HelperBoxWindow::SaveSettings(ini);
}

void ChestRunnerWindow::Draw(IDirect3DDevice9 *pDevice)
{
    UNREFERENCED_PARAMETER(pDevice);

    if (GW::Map::GetInstanceType() == GW::Constants::InstanceType::Loading)
        return;

    if (!visible)
        return;

    const auto size = ImVec2(100.0, 100.0);
    ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);

    if (ImGui::Begin("ChestRunnerWindow", nullptr, GetWinFlags()))
    {
        if (ImGui::Button("Start Helper", size))
        {
            start_helper = !start_helper;

            if (start_helper)
            {
                Log::Log(fmt::format("Started Helper").data());
            }
            else
            {
                Log::Log(fmt::format("Paused Helper").data());
            }
        }
    }

    ImGui::End();
}

bool ChestRunnerWindow::HelperMain()
{
    static bool reset = false;

    if (!start_helper)
    {
        return false;
    }

    if ((action_idx >= actions.size()) || issue_resign)
    {
        action_idx = 0;
        start_helper = true;

        (void)SafeResign(issue_resign);
        issue_resign = false;

        used_targets.clear();
    }

    const bool blocked = DetectNotMoving(20);

    auto action_fn = actions[action_idx];
    const auto state = (*action_fn)(reset);

    if (IsExplorable() && types[action_idx] == ActionType::WALK)
    {
        RunnerSkillLogic();

        reset = true;
    }
    else if (blocked)
    {
        reset = true;
    }
    else
    {
        reset = false;
    }

    const GW::Agent *const target = GW::Agents::GetTarget();

    if (IsExplorable() && types[action_idx] == ActionType::CHANGE_TARGET)
    {
        if (target)
        {
            if (!used_targets.contains(target->agent_id))
            {
                used_targets.insert(target->agent_id);
            }
            else
            {
                ++action_idx;
            }
        }
    }

    if (ActionState::FINISHED == state)
    {
        ++action_idx;
    }

    return true;
}

void ChestRunnerWindow::RunnerSkillLogic()
{
    const auto me = GW::Agents::GetPlayer();

    if (nullptr == me)
        return;

    const auto skills = GW::SkillbarMgr::GetPlayerSkillbar()->skills;
    if (nullptr == skills)
        return;

    skillbar.Update(skills);

    const auto energy_tpl = GetEnergy();
    const auto energy = std::get<0>(energy_tpl);
    const auto hp_tpl = GetHp();
    const auto hp_perc = std::get<2>(hp_tpl);

    if (skillbar.dwarfen.recharge == 0 && energy > skillbar.dwarfen.energy_cost)
    {
        SafeUseSkill(skillbar.dwarfen.idx);
        return;
    }

    if (skillbar.dash.recharge == 0 && energy > skillbar.dash.energy_cost)
    {
        SafeUseSkill(skillbar.dash.idx);
        return;
    }
    else if (skillbar.dash.recharge >= 3 && skillbar.dark_escape.recharge == 0 &&
             energy > skillbar.dark_escape.energy_cost)
    {
        SafeUseSkill(skillbar.dark_escape.idx);
        return;
    }

    if (hp_perc < 0.75 && energy > skillbar.shroud.energy_cost)
    {
        SafeUseSkill(skillbar.shroud.idx);
        return;
    }

    if (skillbar.sf.recharge == 0 && energy > skillbar.sf.energy_cost)
    {
        SafeUseSkill(skillbar.sf.idx);
        return;
    }

    const auto me_living = me->GetAsAgentLiving();

    if (!me_living)
        return;

    const auto need_iau = me_living->GetIsCrippled();

    if (need_iau && skillbar.iau.recharge == 0 && energy > skillbar.iau.energy_cost)
    {
        SafeUseSkill(skillbar.iau.idx);
        return;
    }
}

void ChestRunnerWindow::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);

    issue_resign = DetectPlayerIsDead();

    if (start_helper)
    {
        HelperMain();
    }
}

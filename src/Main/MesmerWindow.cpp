#include "stdafx.h"

#include <array>
#include <cstdint>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/Context/GameContext.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Player.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/EffectMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/PlayerMgr.h>
#include <GWCA/Managers/SkillbarMgr.h>
#include <GWCA/Packets/Opcodes.h>

#include <fmt/format.h>

#include <HelperBox.h>
#include <Logger.h>
#include <Timer.h>

#include <Actions.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <Player.h>
#include <Skillbars.h>
#include <Types.h>
#include <Utils.h>

#include "MesmerWindow.h"

namespace
{
static const auto DEFAULT_WINDOW_SIZE = ImVec2(100.0F, 100.0F);
static constexpr auto MAX_TABLE_LENGTH = 6U;
} // namespace

RoutineState SpikeSet::Routine()
{
    static uint32_t state_idx = 0;

    if (!player->CanCast())
    {
        return RoutineState::ACTIVE;
    }

    if (!player->target || player->target->type != static_cast<uint32_t>(GW::Constants::AgentType::Living))
    {
        return RoutineState::FINISHED;
    }

    const auto target_living = player->target->GetAsAgentLiving();

    if (target_living->GetIsDead())
    {
        return RoutineState::FINISHED;
    }

    if (state_idx == 0)
    {
        (void)SafeUseSkill(skillbar->demise.idx, player->target->agent_id);
        ++state_idx;
        return RoutineState::ACTIVE;
    }
    else if (state_idx == 1)
    {
        (void)SafeUseSkill(skillbar->worry.idx, player->target->agent_id);
        ++state_idx;
        return RoutineState::ACTIVE;
    }

    state_idx = 0;
    return RoutineState::FINISHED;
}

void SpikeSet::Update()
{
    if (action_state == ActionState::ACTIVE)
    {
        const auto routine_state = Routine();

        if (routine_state == RoutineState::FINISHED)
        {
            action_state = ActionState::INACTIVE;
        }
    }
}

void MesmerWindow::Draw(IDirect3DDevice9 *pDevice)
{
    UNREFERENCED_PARAMETER(pDevice);

    if (IsLoading())
        return;

    if (!visible)
        return;

    if (IsExplorable() && GW::Map::GetMapID() != GW::Constants::MapID::The_Underworld)
        return;

    ImGui::SetNextWindowSize(DEFAULT_WINDOW_SIZE, ImGuiCond_FirstUseEver);

    if (ImGui::Begin("MesmerWindow", nullptr, GetWinFlags()))
    {
        spike_set.Draw(ImVec2(ImGui::GetWindowWidth(), 35.0F));

        uint32_t idx = 0;
        for (const auto &foe : filtered_foes)
        {
            bool pushed = false;
            if (foe->hp == 0.0F)
                continue;

            if (foe->GetIsHexed())
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8F, 0.0F, 0.2F, 1.0));
                pushed = true;
            }
            const float distance = GW::GetDistance(player.pos, foe->pos);
            ImGui::Text("Aatxe: %3.0f, %4.0f", foe->hp * 100.0F, distance);
            if (pushed)
            {
                ImGui::PopStyleColor();
            }

            ImGui::SameLine();
            const auto label = fmt::format("Target##{}", idx);
            if (ImGui::Button(label.data()))
            {
                player.ChangeTarget(foe->agent_id);
            }

            ++idx;

            if (idx >= MAX_TABLE_LENGTH)
            {
                break;
            }
        }
    }

    ImGui::End();
}

void MesmerWindow::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);

    if (!player.ValidateData())
        return;
    player.Update();

    if (!skillbar.ValidateData())
        return;
    skillbar.Update();

    spike_set.Update();

    filtered_foes.clear();
    auto agents_array = GW::Agents::GetAgentArray();
    FilterAgents(player,
                 agents_array,
                 filtered_foes,
                 GW::Constants::ModelID::UW::BladedAatxe,
                 GW::Constants::Range::Spirit);
    SortByDistance(player, filtered_foes);
}

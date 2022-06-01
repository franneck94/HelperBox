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
static constexpr auto MAX_TABLE_LENGTH = 6U;

static const auto IDS = std::array<uint32_t, 3>{GW::Constants::ModelID::UW::BladedAatxe,
                                                GW::Constants::ModelID::UW::SkeletonOfDhuum1,
                                                GW::Constants::ModelID::UW::SkeletonOfDhuum2};
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
    if (!target_living || target_living->GetIsDead())
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

    if ((IsExplorable() && !IsUw()) || (IsOutpost() && !IsUwEntryOutpost()))
        return;

    ImGui::SetNextWindowSize(ImVec2(100.0F, 100.0F), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("MesmerWindow", nullptr, GetWinFlags()))
    {
        const auto width = ImGui::GetWindowWidth();
        spike_set.Draw(ImVec2(width, 35.0F));

        if (ImGui::BeginTable("AatxeTable", 3))
        {
            ImGui::TableSetupColumn("HP", ImGuiTableColumnFlags_WidthFixed, width * 0.3);
            ImGui::TableSetupColumn("Dist.", ImGuiTableColumnFlags_WidthFixed, width * 0.25);
            ImGui::TableSetupColumn("Target", ImGuiTableColumnFlags_WidthFixed, width * 0.45);

            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            ImGui::TableNextColumn();
            ImGui::Text("HP");
            ImGui::TableNextColumn();
            ImGui::Text("Dist.");
            ImGui::TableNextColumn();
            ImGui::Text("Target");

            uint32_t idx = 0;
            for (const auto &foe : filtered_foes)
            {
                if (!foe)
                    continue;

                ImGui::TableNextRow();

                bool pushed = false;
                if (foe->hp == 0.0F)
                    continue;

                if (foe->GetIsHexed())
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8F, 0.0F, 0.2F, 1.0));
                    pushed = true;
                }
                else if (foe->player_number == static_cast<uint32_t>(GW::Constants::ModelID::UW::SkeletonOfDhuum1) ||
                         foe->player_number == static_cast<uint32_t>(GW::Constants::ModelID::UW::SkeletonOfDhuum2))
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1F, 0.8F, 0.9F, 1.0));
                    pushed = true;
                }
                const float distance = GW::GetDistance(player.pos, foe->pos);
                ImGui::TableNextColumn();
                ImGui::Text("%3.0f%%", foe->hp * 100.0F);
                ImGui::TableNextColumn();
                ImGui::Text("%4.0f", distance);
                if (pushed)
                    ImGui::PopStyleColor();
                const auto label = fmt::format("Target##{}", idx);
                ImGui::TableNextColumn();
                if (ImGui::Button(label.data()))
                    player.ChangeTarget(foe->agent_id);

                ++idx;

                if (idx >= MAX_TABLE_LENGTH)
                    break;
            }
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

void MesmerWindow::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);

    if (!IsExplorable())
        return;

    filtered_foes.clear();

    if (!player.ValidateData())
        return;
    player.Update();

    if (!skillbar.ValidateData())
        return;
    skillbar.Update();

    spike_set.Update();

    auto agents_array = GW::Agents::GetAgentArray();
    FilterAgents(player, agents_array, filtered_foes, IDS, GW::Constants::Range::Spirit);
    SortByDistance(player, filtered_foes);
}

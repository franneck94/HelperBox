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

#include "TerraWindow.h"

namespace
{
static constexpr auto HEALING_SPRING_U16 = static_cast<uint16_t>(GW::Constants::SkillID::Healing_Spring);
static constexpr auto MIN_IDLE_TIME_S = 0.1F;
static constexpr auto MAX_TABLE_LENGTH = 6U;
static auto auto_target_active = false;

static const auto IDS = std::array<uint32_t, 3>{GW::Constants::ModelID::UW::ObsidianBehemoth,
                                                GW::Constants::ModelID::UW::SkeletonOfDhuum1,
                                                GW::Constants::ModelID::UW::SkeletonOfDhuum2};
} // namespace

void AutoTargetAction::Update()
{
    if (!IsExplorable())
    {
        auto_target_active = false;
        action_state = ActionState::INACTIVE;
    }

    if (action_state == ActionState::ACTIVE)
    {
        auto_target_active = true;
    }
    else
    {
        auto_target_active = false;
        action_state = ActionState::INACTIVE;
    }
}

RoutineState AutoTargetAction::Routine()
{
    return RoutineState::NONE;
}

void TerraWindow::Draw(IDirect3DDevice9 *pDevice)
{
    UNREFERENCED_PARAMETER(pDevice);

    if (IsLoading())
        return;

    if (!visible)
        return;

    if ((IsExplorable() && !IsUw()) || (IsOutpost() && !IsUwEntryOutpost()))
        return;

    ImGui::SetNextWindowSize(ImVec2(100.0F, 100.0F), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("TerraWindow", nullptr, GetWinFlags()))
    {
        const auto width = ImGui::GetWindowWidth();
        auto_target.Draw(ImVec2(width, 35.0F));

        if (ImGui::BeginTable("BehemothTable", 4))
        {
            ImGui::TableSetupColumn("HP", ImGuiTableColumnFlags_WidthFixed, width * 0.15);
            ImGui::TableSetupColumn("Dist.", ImGuiTableColumnFlags_WidthFixed, width * 0.2);
            ImGui::TableSetupColumn("Cast.", ImGuiTableColumnFlags_WidthFixed, width * 0.2);
            ImGui::TableSetupColumn("Target", ImGuiTableColumnFlags_WidthFixed, width * 0.4);

            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            ImGui::TableNextColumn();
            ImGui::Text("HP");
            ImGui::TableNextColumn();
            ImGui::Text("Dist.");
            ImGui::TableNextColumn();
            ImGui::Text("Cast.");
            ImGui::TableNextColumn();
            ImGui::Text("Target");

            uint32_t idx = 0;
            for (const auto &foe : filtered_foes)
            {
                if (!foe)
                    continue;

                ImGui::TableNextRow();

                bool pushed = false;
                if (foe->GetIsDead())
                    continue;

                if (foe->GetIsCasting() && foe->skill == HEALING_SPRING_U16)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1F, 0.9F, 0.1F, 1.0));
                    pushed = true;

                    last_casted_times_ms[foe->agent_id] = clock();
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
                ImGui::TableNextColumn();
                const auto timer_diff_ms = TIMER_DIFF(last_casted_times_ms[foe->agent_id]);
                const auto timer_diff_s = timer_diff_ms / 1000;
                if (timer_diff_s > 40)
                {
                    ImGui::Text(" - ");
                }
                else
                {
                    ImGui::Text("%2ds", timer_diff_s);
                }
                if (pushed)
                {
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
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
        ImGui::EndTable();
    }
    ImGui::End();
}

void TerraWindow::Update(float delta)
{
    if (!IsExplorable())
        return;

    filtered_foes.clear();

    if (IsLoading())
        last_casted_times_ms.clear();

    if (!player.ValidateData())
        return;

    player.Update();
    auto_target.Update();

    auto agents_array = GW::Agents::GetAgentArray();
    FilterAgents(player, agents_array, filtered_foes, IDS, GW::Constants::Range::Spellcast);
    SortByDistance(player, filtered_foes);

    if (!auto_target_active)
        return;

    for (const auto &foe : filtered_foes)
    {
        if (!foe)
            continue;

        const auto dist = GW::GetDistance(player.pos, foe->pos);

        if (dist < GW::Constants::Range::Earshot && foe->GetIsCasting() && foe->skill == HEALING_SPRING_U16)
        {
            player.ChangeTarget(foe->agent_id);
        }
    }
}

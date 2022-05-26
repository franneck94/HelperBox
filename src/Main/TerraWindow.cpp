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
static const auto DEFAULT_WINDOW_SIZE = ImVec2(100.0F, 100.0F);
static constexpr auto HEALING_SPRING_U16 = static_cast<uint16_t>(GW::Constants::SkillID::Healing_Spring);
static constexpr auto MIN_IDLE_TIME_S = 0.1F;
static constexpr auto MAX_TABLE_LENGTH = 6U;
static auto auto_target_active = false;
} // namespace

void AutoTargetAction::Update()
{
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

    if (IsExplorable() && GW::Map::GetMapID() != GW::Constants::MapID::The_Underworld)
        return;

    ImGui::SetNextWindowSize(DEFAULT_WINDOW_SIZE, ImGuiCond_FirstUseEver);

    if (ImGui::Begin("TerraWindow", nullptr, GetWinFlags()))
    {
        const auto width = ImGui::GetWindowWidth();
        auto_target.Draw(ImVec2(width, 35.0F));

        if (ImGui::BeginTable("BehemothTable", 4))
        {
            ImGui::TableSetupColumn("HP (%%)", ImGuiTableColumnFlags_WidthFixed, width * 0.2);
            ImGui::TableSetupColumn("Dist.", ImGuiTableColumnFlags_WidthFixed, width * 0.15);
            ImGui::TableSetupColumn("Cast. (s)", ImGuiTableColumnFlags_WidthFixed, width * 0.25);
            ImGui::TableSetupColumn("Target", ImGuiTableColumnFlags_WidthFixed, width * 0.4);

            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            ImGui::TableNextColumn();
            ImGui::Text("HP (%%)");
            ImGui::TableNextColumn();
            ImGui::Text("Dist.");
            ImGui::TableNextColumn();
            ImGui::Text("Cast. (s)");
            ImGui::TableNextColumn();
            ImGui::Text("Target");

            uint32_t idx = 0;
            for (const auto &foe : filtered_foes)
            {
                ImGui::TableNextRow();

                bool pushed = false;
                if (foe->GetIsDead())
                    continue;

                if (foe->GetIsCasting() && foe->skill == HEALING_SPRING_U16)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1F, 0.9F, 0.1F, 1.0));
                    pushed = true;
                }
                const float distance = GW::GetDistance(player.pos, foe->pos);
                ImGui::TableNextColumn();
                ImGui::Text("%3.0f", foe->hp * 100.0F);
                ImGui::TableNextColumn();
                ImGui::Text("%4.0f", distance);
                ImGui::TableNextColumn();
                ImGui::Text("%2.0f", last_casted_times_ms[foe->agent_id]);
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
            ImGui::EndTable();
        }
        ImGui::End();
    }
}

void TerraWindow::Update(float delta)
{
    static float idle_time_ms = 0;

    filtered_foes.clear();
    last_casted_times_ms.clear();

    if (!player.ValidateData())
        return;

    player.Update();
    auto_target.Update();

    auto agents_array = GW::Agents::GetAgentArray();
    FilterAgents(player,
                 agents_array,
                 filtered_foes,
                 GW::Constants::ModelID::UW::ObsidianBehemoth,
                 GW::Constants::Range::Spellcast);
    SortByDistance(player, filtered_foes);

    for (const auto &foe : filtered_foes)
    {
        last_casted_times_ms[foe->agent_id] = 0.0F;
    }

    if (!player.living->GetIsMoving())
    {
        idle_time_ms += delta;
    }
    else
    {
        idle_time_ms = 0.0F;
    }

    if (!auto_target_active)
        return;

    if (filtered_foes.size() == 0)
        return;

    const auto nearest_behemoth = filtered_foes[0];
    const auto dist_nearest = GW::GetDistance(player.pos, nearest_behemoth->pos);

    if (idle_time_ms > MIN_IDLE_TIME_S && !player.living->GetIsMoving() && dist_nearest < GW::Constants::Range::Area)
    {
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
}

#include "stdafx.h"

#include <array>
#include <cstdint>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Player.h>
#include <GWCA/Managers/PartyMgr.h>

#include <Base/HelperBox.h>

#include <GuiUtils.h>
#include <Logger.h>

#include <Actions.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <MathUtils.h>
#include <PlayerData.h>
#include <Types.h>
#include <UwHelper.h>

#include <fmt/format.h>

#include "MainteamWindow.h"

namespace
{
static constexpr auto MAX_TABLE_LENGTH = 6U;

static const auto IDS = std::array<uint32_t, 6>{GW::Constants::ModelID::UW::BladedAatxe,
                                                GW::Constants::ModelID::UW::DyingNightmare,
                                                GW::Constants::ModelID::UW::TerrorwebDryder,
                                                GW::Constants::ModelID::UW::FourHorseman,
                                                GW::Constants::ModelID::UW::SkeletonOfDhuum1,
                                                GW::Constants::ModelID::UW::SkeletonOfDhuum2};
} // namespace

void MainteamWindow::DrawSplittedAgents(std::vector<GW::AgentLiving *> livings,
                                        const ImVec4 color,
                                        std::string_view label)
{
    auto idx = uint32_t{0};

    for (const auto living : livings)
    {
        if (!living)
            continue;

        ImGui::TableNextRow();

        if (living->hp == 0.0F || living->GetIsDead())
            continue;

        if ((living->player_number == static_cast<uint32_t>(GW::Constants::ModelID::UW::BladedAatxe) ||
             living->player_number == static_cast<uint32_t>(GW::Constants::ModelID::UW::FourHorseman)) &&
            living->GetIsHexed())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8F, 0.0F, 0.2F, 1.0F));
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Text, color);
        }
        const auto distance = GW::GetDistance(player_data.pos, living->pos);
        ImGui::TableNextColumn();
        ImGui::Text("%3.0f%%", living->hp * 100.0F);
        ImGui::TableNextColumn();
        ImGui::Text("%4.0f", distance);
        ImGui::PopStyleColor();

        const auto _label = fmt::format("Target##{}{}", label.data(), idx);
        ImGui::TableNextColumn();
        if (ImGui::Button(_label.data()))
            player_data.ChangeTarget(living->agent_id);

        ++idx;

        if (idx >= MAX_TABLE_LENGTH)
            break;
    }
}

void MainteamWindow::Draw(IDirect3DDevice9 *)
{
    if (!visible)
        return;

    if (!UwHelperActivationConditions())
        return;
    if (!IsSpiker(player_data) && !IsLT(player_data))
        return;

    ImGui::SetNextWindowSize(ImVec2(200.0F, 240.0F), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("MainteamWindow", nullptr, GetWinFlags() | ImGuiWindowFlags_NoScrollbar))
    {
        const auto width = ImGui::GetWindowWidth();

        if (ImGui::BeginTable("AatxeTable", 3))
        {
            ImGui::TableSetupColumn("HP", ImGuiTableColumnFlags_WidthFixed, width * 0.27F);
            ImGui::TableSetupColumn("Dist.", ImGuiTableColumnFlags_WidthFixed, width * 0.25F);
            ImGui::TableSetupColumn("Target", ImGuiTableColumnFlags_WidthFixed, width * 0.48F);

            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            ImGui::TableNextColumn();
            ImGui::Text("HP");
            ImGui::TableNextColumn();
            ImGui::Text("Dist.");
            ImGui::TableNextColumn();
            ImGui::Text("Target");

            DrawSplittedAgents(horseman_livings, ImVec4(0.568F, 0.239F, 1.0F, 1.0F), "Horseman");
            DrawSplittedAgents(aatxe_livings, ImVec4(1.0F, 1.0F, 1.0F, 1.0F), "Aatxe");
            DrawSplittedAgents(nightmare_livings, ImVec4(0.6F, 0.4F, 1.0F, 1.0F), "Nightmare");
            DrawSplittedAgents(keeper_livings, ImVec4(0.90F, 0.35F, 0.09F, 1.0F), "Keeper");
            DrawSplittedAgents(dryder_livings, ImVec4(0.94F, 0.31F, 0.09F, 1.0F), "Dryder");
            DrawSplittedAgents(skele_livings, ImVec4(0.1F, 0.8F, 0.9F, 1.0F), "Skele");
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

void MainteamWindow::Update(float)
{
    filtered_livings.clear();
    aatxe_livings.clear();
    nightmare_livings.clear();
    dryder_livings.clear();
    skele_livings.clear();
    horseman_livings.clear();
    keeper_livings.clear();

    if (!player_data.ValidateData(UwHelperActivationConditions))
        return;
    player_data.Update();

    if (!IsSpiker(player_data) && !IsLT(player_data))
        return;

    FilterAgents(player_data, filtered_livings, IDS, GW::Constants::Allegiance::Enemy, 1600.0F);
    SplitFilteredAgents(filtered_livings, aatxe_livings, GW::Constants::ModelID::UW::BladedAatxe);
    SplitFilteredAgents(filtered_livings, nightmare_livings, GW::Constants::ModelID::UW::DyingNightmare);
    SplitFilteredAgents(filtered_livings, dryder_livings, GW::Constants::ModelID::UW::TerrorwebDryder);
    SplitFilteredAgents(filtered_livings, horseman_livings, GW::Constants::ModelID::UW::FourHorseman);
    SplitFilteredAgents(filtered_livings, keeper_livings, GW::Constants::ModelID::UW::KeeperOfSouls);
    SplitFilteredAgents(filtered_livings, skele_livings, GW::Constants::ModelID::UW::SkeletonOfDhuum1);
    SplitFilteredAgents(filtered_livings, skele_livings, GW::Constants::ModelID::UW::SkeletonOfDhuum2);
    SortByDistance(player_data, aatxe_livings);
    SortByDistance(player_data, nightmare_livings);
    SortByDistance(player_data, horseman_livings);
    SortByDistance(player_data, keeper_livings);
    SortByDistance(player_data, dryder_livings);
    SortByDistance(player_data, skele_livings);
}

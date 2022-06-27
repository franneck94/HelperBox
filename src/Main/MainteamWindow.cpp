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

#include <fmt/format.h>

#include <GuiUtils.h>
#include <HelperBox.h>
#include <Logger.h>

#include <Actions.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <MathUtils.h>
#include <Player.h>
#include <Types.h>
#include <UwHelper.h>

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
        const auto distance = GW::GetDistance(player.pos, living->pos);
        ImGui::TableNextColumn();
        ImGui::Text("%3.0f%%", living->hp * 100.0F);
        ImGui::TableNextColumn();
        ImGui::Text("%4.0f", distance);
        ImGui::PopStyleColor();

        const auto _label = fmt::format("Target##{}{}", label.data(), idx);
        ImGui::TableNextColumn();
        if (ImGui::Button(_label.data()))
            player.ChangeTarget(living->agent_id);

        ++idx;

        if (idx >= MAX_TABLE_LENGTH)
            break;
    }
}

void MainteamWindow::Draw(IDirect3DDevice9 *pDevice)
{
    UNREFERENCED_PARAMETER(pDevice);

    if (!visible)
        return;

    if (!UwHelperActivationConditions())
        return;
    if (!IsDhuumBitch(player) && !IsSpiker(player) && !IsLT(player) && !IsEmo(player))
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
            DrawSplittedAgents(dryder_livings, ImVec4(0.94F, 0.31F, 0.09F, 1.0F), "Dryder");
            DrawSplittedAgents(skele_livings, ImVec4(0.1F, 0.8F, 0.9F, 1.0F), "Skele");
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

void MainteamWindow::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);

    filtered_livings.clear();
    aatxe_livings.clear();
    nightmare_livings.clear();
    dryder_livings.clear();
    skele_livings.clear();
    horseman_livings.clear();

    if (!player.ValidateData(UwHelperActivationConditions))
        return;
    player.Update();

    if (!IsDhuumBitch(player) && !IsSpiker(player) && !IsLT(player) && !IsEmo(player))
        return;

    FilterAgents(player,
                 filtered_livings,
                 IDS,
                 GW::Constants::Allegiance::Enemy,
                 GW::Constants::Range::Spellcast + 200.0F);
    SplitFilteredAgents(filtered_livings, aatxe_livings, GW::Constants::ModelID::UW::BladedAatxe);
    SplitFilteredAgents(filtered_livings, nightmare_livings, GW::Constants::ModelID::UW::DyingNightmare);
    SplitFilteredAgents(filtered_livings, dryder_livings, GW::Constants::ModelID::UW::TerrorwebDryder);
    SplitFilteredAgents(filtered_livings, horseman_livings, GW::Constants::ModelID::UW::FourHorseman);
    SplitFilteredAgents(filtered_livings, skele_livings, GW::Constants::ModelID::UW::SkeletonOfDhuum1);
    SplitFilteredAgents(filtered_livings, skele_livings, GW::Constants::ModelID::UW::SkeletonOfDhuum2);
    SortByDistance(player, aatxe_livings);
    SortByDistance(player, nightmare_livings);
    SortByDistance(player, horseman_livings);
    SortByDistance(player, dryder_livings);
    SortByDistance(player, skele_livings);
}

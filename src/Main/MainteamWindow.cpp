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

void MainteamWindow::DrawSplittedAgents(std::vector<GW::AgentLiving *> splitted_agents,
                                        const ImVec4 color,
                                        std::string_view label)
{
    auto idx = uint32_t{0};

    for (const auto &foe : splitted_agents)
    {
        if (!foe)
            continue;

        ImGui::TableNextRow();

        auto pushed = false;
        if (foe->hp == 0.0F)
            continue;

        if (foe->player_number == static_cast<uint32_t>(GW::Constants::ModelID::UW::BladedAatxe) && foe->GetIsHexed())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8F, 0.0F, 0.2F, 1.0F));
            pushed = true;
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            pushed = true;
        }
        const auto distance = GW::GetDistance(player.pos, foe->pos);
        ImGui::TableNextColumn();
        ImGui::Text("%3.0f%%", foe->hp * 100.0F);
        ImGui::TableNextColumn();
        ImGui::Text("%4.0f", distance);
        if (pushed)
            ImGui::PopStyleColor();

        const auto _label = fmt::format("Target##{}{}", label.data(), idx);
        ImGui::TableNextColumn();
        if (ImGui::Button(_label.data()))
            player.ChangeTarget(foe->agent_id);

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

    if (!ActivationConditions())
        return;

    ImGui::SetNextWindowSize(ImVec2(190.0F, 235.0F), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("MainteamWindow", nullptr, GetWinFlags()))
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

            DrawSplittedAgents(horseman_agents, ImVec4(1.0F, 1.0F, 1.0F, 1.0F), "Horseman");
            DrawSplittedAgents(aatxe_agents, ImVec4(1.0F, 1.0F, 1.0F, 1.0F), "Aatxe");
            DrawSplittedAgents(nightmare_agents, ImVec4(0.6F, 0.4F, 1.0F, 1.0F), "Nightmare");
            DrawSplittedAgents(dryder_agents, ImVec4(0.94F, 0.31F, 0.09F, 1.0F), "Dryder");
            DrawSplittedAgents(skele_agents, ImVec4(0.1F, 0.8F, 0.9F, 1.0F), "Skele");
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

void MainteamWindow::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);

    filtered_foes.clear();
    aatxe_agents.clear();
    nightmare_agents.clear();
    dryder_agents.clear();
    skele_agents.clear();
    horseman_agents.clear();

    if (!player.ValidateData())
        return;
    player.Update();

    if (!ActivationConditions())
        return;

    const auto agents_array = GW::Agents::GetAgentArray();
    FilterAgents(player,
                 agents_array,
                 filtered_foes,
                 IDS,
                 GW::Constants::Allegiance::Enemy,
                 GW::Constants::Range::Spellcast + 200.0F);
    SplitFilteredAgents(filtered_foes, aatxe_agents, GW::Constants::ModelID::UW::BladedAatxe);
    SplitFilteredAgents(filtered_foes, nightmare_agents, GW::Constants::ModelID::UW::DyingNightmare);
    SplitFilteredAgents(filtered_foes, dryder_agents, GW::Constants::ModelID::UW::TerrorwebDryder);
    SplitFilteredAgents(filtered_foes, horseman_agents, GW::Constants::ModelID::UW::FourHorseman);
    SplitFilteredAgents(filtered_foes, skele_agents, GW::Constants::ModelID::UW::SkeletonOfDhuum1);
    SplitFilteredAgents(filtered_foes, skele_agents, GW::Constants::ModelID::UW::SkeletonOfDhuum2);
    SortByDistance(player, aatxe_agents);
    SortByDistance(player, nightmare_agents);
    SortByDistance(player, horseman_agents);
    SortByDistance(player, dryder_agents);
    SortByDistance(player, skele_agents);
}

bool MainteamWindow::ActivationConditions()
{
    if (player.primary != GW::Constants::Profession::Mesmer && player.primary != GW::Constants::Profession::Ritualist &&
        player.primary != GW::Constants::Profession::Dervish &&
        player.primary != GW::Constants::Profession::Elementalist)
        return false;

    if (!GW::PartyMgr::GetIsPartyLoaded())
        return false;

    if (IsUwEntryOutpost() || IsUw())
        return true;

    return false;
}

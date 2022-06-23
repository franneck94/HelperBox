#include "stdafx.h"

#include <array>
#include <cstdint>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Player.h>
#include <GWCA/Managers/PartyMgr.h>

#include <fmt/format.h>

#include <HelperBox.h>
#include <Timer.h>

#include <Actions.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <MathUtils.h>
#include <Player.h>
#include <Skillbars.h>
#include <Types.h>
#include <UwHelper.h>

#include "TerraWindow.h"

namespace
{
static constexpr auto HEALING_SPRING_U16 = static_cast<uint16_t>(GW::Constants::SkillID::Healing_Spring);
static constexpr auto MAX_TABLE_LENGTH = 6U;
static auto auto_target_active = false;

static const auto T2_IDS = std::array<uint32_t, 1>{GW::Constants::ModelID::UW::ObsidianBehemoth};
static const auto GENERAL_IDS = std::array<uint32_t, 4>{GW::Constants::ModelID::UW::TerrorwebDryder,
                                                        GW::Constants::ModelID::UW::FourHorseman,
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

void TerraWindow::DrawSplittedAgents(std::vector<GW::AgentLiving *> filtered_livings,
                                     const ImVec4 color,
                                     std::string_view label)
{
    auto idx = uint32_t{0};

    for (const auto living : filtered_livings)
    {
        if (!living)
            continue;

        ImGui::TableNextRow();

        if (living->hp == 0.0F || living->GetIsDead())
            continue;

        if (living->player_number == static_cast<uint32_t>(GW::Constants::ModelID::UW::ObsidianBehemoth) &&
            living->GetIsCasting() && living->skill == HEALING_SPRING_U16)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1F, 0.9F, 0.1F, 1.0));
            last_casted_times_ms[living->agent_id] = clock();
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
        ImGui::TableNextColumn();
        const auto timer_diff_ms = TIMER_DIFF(last_casted_times_ms[living->agent_id]);
        const auto timer_diff_s = timer_diff_ms / 1000;
        if (timer_diff_s > 40)
        {
            ImGui::Text(" - ");
        }
        else
        {
            ImGui::Text("%2ds", timer_diff_s);
        }
        ImGui::PopStyleColor();

        ImGui::TableNextColumn();
        const auto _label = fmt::format("Target##{}{}", label.data(), idx);
        if (ImGui::Button(_label.data()))
            player.ChangeTarget(living->agent_id);

        ++idx;

        if (idx >= MAX_TABLE_LENGTH)
            break;
    }
}

void TerraWindow::Draw(IDirect3DDevice9 *pDevice)
{
    UNREFERENCED_PARAMETER(pDevice);

    if (!visible)
        return;

    if (!ActivationConditions())
        return;

    ImGui::SetNextWindowSize(ImVec2(200.0F, 240.0F), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("TerraWindow", nullptr, GetWinFlags()))
    {
        const auto width = ImGui::GetWindowWidth();
        auto_target.Draw(ImVec2(width, 35.0F));

        if (ImGui::BeginTable("BehemothTable", 4))
        {
            ImGui::TableSetupColumn("HP", ImGuiTableColumnFlags_WidthFixed, width * 0.15F);
            ImGui::TableSetupColumn("Dist.", ImGuiTableColumnFlags_WidthFixed, width * 0.2F);
            ImGui::TableSetupColumn("Cast.", ImGuiTableColumnFlags_WidthFixed, width * 0.2F);
            ImGui::TableSetupColumn("Target", ImGuiTableColumnFlags_WidthFixed, width * 0.4F);

            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            ImGui::TableNextColumn();
            ImGui::Text("HP");
            ImGui::TableNextColumn();
            ImGui::Text("Dist.");
            ImGui::TableNextColumn();
            ImGui::Text("Cast.");
            ImGui::TableNextColumn();
            ImGui::Text("Target");

            DrawSplittedAgents(horseman_livings, ImVec4(0.568F, 0.239F, 1.0F, 1.0F), "Horseman");
            DrawSplittedAgents(behemoth_livings, ImVec4(1.0F, 1.0F, 1.0F, 1.0F), "Behemoth");
            DrawSplittedAgents(dryder_livings, ImVec4(0.94F, 0.31F, 0.09F, 1.0F), "Dryder");
            DrawSplittedAgents(skele_livings, ImVec4(0.1F, 0.8F, 0.9F, 1.0F), "Skele");
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

void TerraWindow::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);

    filtered_foes.clear();
    behemoth_livings.clear();
    dryder_livings.clear();
    skele_livings.clear();
    horseman_livings.clear();

    if (IsLoading())
        last_casted_times_ms.clear();

    if (!player.ValidateData())
        return;
    player.Update();

    if (!ActivationConditions())
        return;

    auto_target.Update();

    auto agents_array = GW::Agents::GetAgentArray();
    FilterAgents(player, agents_array, filtered_foes, T2_IDS, GW::Constants::Allegiance::Enemy, 800.0F);
    FilterAgents(player, agents_array, filtered_foes, GENERAL_IDS, GW::Constants::Allegiance::Enemy, 1500.0F);
    SplitFilteredAgents(filtered_foes, behemoth_livings, GW::Constants::ModelID::UW::ObsidianBehemoth);
    SplitFilteredAgents(filtered_foes, dryder_livings, GW::Constants::ModelID::UW::TerrorwebDryder);
    SplitFilteredAgents(filtered_foes, skele_livings, GW::Constants::ModelID::UW::SkeletonOfDhuum1);
    SplitFilteredAgents(filtered_foes, skele_livings, GW::Constants::ModelID::UW::SkeletonOfDhuum2);
    SplitFilteredAgents(filtered_foes, horseman_livings, GW::Constants::ModelID::UW::FourHorseman);
    SortByDistance(player, behemoth_livings);
    SortByDistance(player, dryder_livings);
    SortByDistance(player, skele_livings);
    SortByDistance(player, horseman_livings);

    if (!auto_target_active)
        return;

    for (const auto &living : behemoth_livings)
    {
        if (!living)
            continue;

        const auto dist = GW::GetDistance(player.pos, living->pos);

        if (dist < GW::Constants::Range::Earshot && living->GetIsCasting() && living->skill == HEALING_SPRING_U16)
            player.ChangeTarget(living->agent_id);
    }
}

bool TerraWindow::ActivationConditions()
{
    if (!GW::Map::GetIsMapLoaded())
        return false;

    if (!GW::PartyMgr::GetIsPartyLoaded())
        return false;

    if (!IsRangerTerra(player) && !IsMesmerTerra(player))
        return false;

    if (IsUwEntryOutpost() || IsUw())
        return true;

    return false;
}

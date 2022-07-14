#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Player.h>
#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/PartyMgr.h>

#include <ActionsBase.h>
#include <ActionsUw.h>
#include <Base/HelperBox.h>
#include <DataPlayer.h>
#include <DataSkillbar.h>
#include <HelperAgents.h>
#include <HelperMaps.h>
#include <HelperUw.h>
#include <HelperUwPos.h>
#include <Utils.h>
#include <UtilsGui.h>
#include <UtilsMath.h>

#include <fmt/format.h>

#include "UwRanger.h"

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

void UwRanger::DrawSplittedAgents(std::vector<GW::AgentLiving *> livings, const ImVec4 color, std::string_view label)
{
    auto idx = uint32_t{0};

    for (const auto living : livings)
    {
        if (!living)
            continue;

        ImGui::TableNextRow();

        if (living->hp == 0.0F || living->GetIsDead())
            continue;

        if (living->player_number == static_cast<uint32_t>(GW::Constants::ModelID::UW::ObsidianBehemoth) &&
            living->GetIsCasting() && living->skill == HEALING_SPRING_U16)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1F, 0.9F, 0.1F, 1.0F));
            last_casted_times_ms[living->agent_id] = clock();
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
            player_data.ChangeTarget(living->agent_id);

        ++idx;

        if (idx >= MAX_TABLE_LENGTH)
            break;
    }
}

void UwRanger::Draw()
{
    if (!visible)
        return;

    if (!UwHelperActivationConditions())
        return;
    if (!IsRangerTerra(player_data))
        return;

    ImGui::SetNextWindowSize(ImVec2(200.0F, 240.0F), ImGuiCond_FirstUseEver);

    if (ImGui::Begin(Name(), nullptr, GetWinFlags() | ImGuiWindowFlags_NoScrollbar))
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

void UwRanger::Update(float, const AgentLivingData &livings_data)
{
    static auto died_just_now = false;
    static auto send_message = false;

    filtered_livings.clear();
    behemoth_livings.clear();
    dryder_livings.clear();
    skele_livings.clear();
    horseman_livings.clear();

    if (IsLoading())
    {
        send_message = false;
        died_just_now = false;
        last_casted_times_ms.clear();
    }

    if (!player_data.ValidateData(UwHelperActivationConditions))
        return;
    player_data.Update();
    if (!send_message && !died_just_now && player_data.dead)
        died_just_now = true;
    if (!send_message && died_just_now && IsAtHeuchlerPattrick(player_data.pos))
    {
        GW::Chat::SendChat('#', L"Oh my god, step bro im stuck. Can you help me?");
        died_just_now = false;
        send_message = true;
    }

    if (!IsRangerTerra(player_data) && !IsMesmerTerra(player_data))
        return;

    auto_target.Update();

    const auto &pos = player_data.pos;
    FilterByIdsAndDistances(pos, livings_data.enemies, filtered_livings, T2_IDS, 800.0F);
    FilterByIdsAndDistances(pos, livings_data.enemies, filtered_livings, GENERAL_IDS, 1500.0F);
    FilterByIdAndDistance(pos, filtered_livings, behemoth_livings, GW::Constants::ModelID::UW::ObsidianBehemoth);
    FilterByIdAndDistance(pos, filtered_livings, dryder_livings, GW::Constants::ModelID::UW::TerrorwebDryder);
    FilterByIdAndDistance(pos, filtered_livings, skele_livings, GW::Constants::ModelID::UW::SkeletonOfDhuum1);
    FilterByIdAndDistance(pos, filtered_livings, skele_livings, GW::Constants::ModelID::UW::SkeletonOfDhuum2);
    FilterByIdAndDistance(pos, filtered_livings, horseman_livings, GW::Constants::ModelID::UW::FourHorseman);
    SortByDistance(player_data, behemoth_livings);
    SortByDistance(player_data, dryder_livings);
    SortByDistance(player_data, skele_livings);
    SortByDistance(player_data, horseman_livings);

    if (!auto_target_active)
        return;

    for (const auto living : behemoth_livings)
    {
        if (!living)
            continue;

        const auto dist = GW::GetDistance(player_data.pos, living->pos);

        if (dist < GW::Constants::Range::Earshot && living->GetIsCasting() && living->skill == HEALING_SPRING_U16)
        {
            player_data.ChangeTarget(living->agent_id);
#ifdef _DEBUG
            if (player_data.living->GetIsIdle() && player_data.target)
                AttackAgent(player_data.target);
#endif
        }
    }
}

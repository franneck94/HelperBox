#include <array>
#include <cstdint>
#include <ranges>
#include <string_view>
#include <vector>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Player.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Managers/SkillbarMgr.h>

#include <ActionsBase.h>
#include <ActionsUw.h>
#include <Base/HelperBox.h>
#include <DataPlayer.h>
#include <Helper.h>
#include <HelperAgents.h>
#include <HelperItems.h>
#include <HelperUw.h>
#include <HelperUwPos.h>
#include <Logger.h>
#include <UtilsGui.h>
#include <UtilsMath.h>

#include <fmt/format.h>

#include "UwMesmer.h"

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

bool LtRoutine::CastHexesOnEnemyType(const std::vector<GW::AgentLiving *> &enemies,
                                     uint32_t &last_skill,
                                     uint32_t &last_id,
                                     const bool use_empathy)
{
    if (enemies.end() ==
        std::find_if(enemies.begin(), enemies.end(), [&](const auto &enemy) { return enemy->agent_id == last_id; }))
    {
        last_skill = 0;
        last_id = 0;
    }

    auto max_num_skills = uint32_t{2U};
    if (use_empathy)
        ++max_num_skills;

    for (size_t i = 0; i < enemies.size(); ++i)
    {
        const auto &enemy = enemies[i];
        const auto id = enemy->agent_id;

        if (last_skill == 0 && enemy->GetIsHexed())
            continue;
        else if (last_skill > 0 && last_skill < max_num_skills && id != last_id)
            continue;
        else if (last_skill >= max_num_skills && id == last_id)
        {
            last_skill = 0;
            continue;
        }

        if (last_skill == 0 && RoutineState::FINISHED == skillbar->worry.Cast(player_data->energy, id))
        {
            if (!player_data->target || (player_data->target && player_data->target->agent_id != id))
                player_data->ChangeTarget(id);
            ++last_skill;
            last_id = id;
            return true;
        }
        else if (last_skill == 1 && RoutineState::FINISHED == skillbar->demise.Cast(player_data->energy, id))
        {
            if (!player_data->target || (player_data->target && player_data->target->agent_id != id))
                player_data->ChangeTarget(id);
            ++last_skill;
            last_id = id;
            return true;
        }
        else if (use_empathy && last_skill == 2 &&
                 RoutineState::FINISHED == skillbar->empathy.Cast(player_data->energy, id))
        {
            if (!player_data->target || (player_data->target && player_data->target->agent_id != id))
                player_data->ChangeTarget(id);
            ++last_skill;
            last_id = id;
            return true;
        }
    }

    return false;
}

bool LtRoutine::RoutineSelfEnches(const std::vector<GW::AgentLiving *> &enemies_in_range) const
{
    const auto nightmares = FilterById(enemies_in_range, GW::Constants::ModelID::UW::DyingNightmare);
    const auto dryders = FilterById(enemies_in_range, GW::Constants::ModelID::UW::TerrorwebDryder);

    const auto found_obsi = player_data->HasEffect(GW::Constants::SkillID::Obsidian_Flesh);
    const auto found_stoneflesh = player_data->HasEffect(GW::Constants::SkillID::Stoneflesh_Aura);
    const auto found_visage = player_data->HasEffect(GW::Constants::SkillID::Sympathetic_Visage);
    const auto found_mantra = player_data->HasEffect(GW::Constants::SkillID::Mantra_of_Resolve);

    const auto obsi_duration_s =
        player_data->GetRemainingEffectDuration(GW::Constants::SkillID::Obsidian_Flesh) / 1000.0F;
    const auto stoneflesh_duration_s =
        player_data->GetRemainingEffectDuration(GW::Constants::SkillID::Stoneflesh_Aura) / 1000.0F;
    const auto visage_duration_s =
        player_data->GetRemainingEffectDuration(GW::Constants::SkillID::Sympathetic_Visage) / 1000.0F;
    const auto mantra_duration_s =
        player_data->GetRemainingEffectDuration(GW::Constants::SkillID::Mantra_of_Resolve) / 1000.0F;

    if ((!found_obsi || obsi_duration_s < 3.0F) && (nightmares.size() || dryders.size()) &&
        (RoutineState::FINISHED == skillbar->obsi.Cast(player_data->energy)))
        return true;

    if ((!found_stoneflesh || stoneflesh_duration_s < 3.0F) && enemies_in_range.size() &&
        (RoutineState::FINISHED == skillbar->stoneflesh.Cast(player_data->energy)))
        return true;

    if ((!found_visage || visage_duration_s < 3.0F) && enemies_in_range.size() &&
        (RoutineState::FINISHED == skillbar->visage.Cast(player_data->energy)))
        return true;

    if ((!found_mantra || mantra_duration_s < 3.0F) && enemies_in_range.size() &&
        (RoutineState::FINISHED == skillbar->mantra_of_resolve.Cast(player_data->energy)))
        return true;

    return false;
}

bool LtRoutine::RoutineSpikeBall(const std::vector<GW::AgentLiving *> &enemies_in_range, const auto include_graspings)
{
    const auto nightmares = FilterById(enemies_in_range, GW::Constants::ModelID::UW::DyingNightmare);
    const auto aatxes = FilterById(enemies_in_range, GW::Constants::ModelID::UW::BladedAatxe);
    const auto dryders = FilterById(enemies_in_range, GW::Constants::ModelID::UW::TerrorwebDryder);
    const auto graspings = FilterById(enemies_in_range, GW::Constants::ModelID::UW::GraspingDarkness);

    if (CastHexesOnEnemyType(nightmares, last_nightmare_skill, last_nightmare_id, true))
        return true;
    if (CastHexesOnEnemyType(dryders, last_dryder_skill, last_dryder_id, false))
        return true;
    if (CastHexesOnEnemyType(aatxes, last_aatxe_skill, last_aatxe_id, false))
        return true;
    if (include_graspings && CastHexesOnEnemyType(graspings, last_graspings_skill, last_graspings_id, false))
        return true;

    return false;
}

bool LtRoutine::ReadyForSpike() const
{
    if (player_data->living->GetIsMoving())
        return false;

    return true;
}

RoutineState LtRoutine::Routine()
{
#ifdef _DEBUG
    if (!IsUw())
        return RoutineState::FINISHED;

    if (!ActionABC::HasWaitedLongEnough(500)) // ms
        return RoutineState::ACTIVE;

    const auto enemies = FilterAgentsByRange(livings_data->enemies, *player_data, GW::Constants::Range::Spellcast);
    const auto is_at_spike_pos =
        IsAtChamberSpike(player_data->pos) || IsAtChamberMonuSpike(player_data->pos) || IsAtFusePulls(player_data->pos);

    if (is_at_spike_pos && ReadyForSpike() && RoutineSelfEnches(enemies))
        return RoutineState::ACTIVE;

    if (enemies.size() == 0)
    {
        action_state = ActionState::INACTIVE;
        return RoutineState::FINISHED;
    }

    if (is_at_spike_pos && ReadyForSpike() && !IsInVale(player_data->pos) && RoutineSpikeBall(enemies, false))
        return RoutineState::ACTIVE;
    if (is_at_spike_pos && ReadyForSpike() && IsInVale(player_data->pos) && RoutineSpikeBall(enemies, true))
        return RoutineState::ACTIVE;
#endif

    return RoutineState::FINISHED;
}

void LtRoutine::Update()
{
    static auto paused = false;

    if (GW::PartyMgr::GetIsPartyDefeated())
        action_state = ActionState::INACTIVE;

    if (action_state == ActionState::ACTIVE && PauseRoutine())
    {
        paused = true;
        action_state = ActionState::ON_HOLD;
    }

    if (action_state == ActionState::ON_HOLD && ResumeRoutine())
    {
        paused = false;
        action_state = ActionState::ACTIVE;

        last_nightmare_id = 0U;
        last_nightmare_skill = 0U;
        last_aatxe_id = 0U;
        last_aatxe_skill = 0U;
        last_dryder_id = 0U;
        last_dryder_skill = 0U;
        last_graspings_id = 0U;
        last_graspings_skill = 0U;
    }

    if (action_state == ActionState::ACTIVE)
        (void)Routine();
}

void UwMesmer::DrawSplittedAgents(std::vector<GW::AgentLiving *> livings, const ImVec4 color, std::string_view label)
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

void UwMesmer::Draw(IDirect3DDevice9 *)
{
    if (!visible)
        return;

    if (!UwHelperActivationConditions())
        return;
    if (!IsUwMesmer(player_data))
        return;

    ImGui::SetNextWindowSize(ImVec2(200.0F, 240.0F), ImGuiCond_FirstUseEver);

    if (ImGui::Begin(Name(), nullptr, GetWinFlags() | ImGuiWindowFlags_NoScrollbar))
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

    if (TankIsSoloLT())
    {
        if (ImGui::Begin("LtWindow", nullptr))
            lt_routine.Draw();
        ImGui::End();
    }
}

void UwMesmer::Update(float, const AgentLivingData &_livings_data)
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

    if (TankIsSoloLT())
    {
        if (!skillbar.ValidateData())
            return;
        skillbar.Update();
    }

    const auto &pos = player_data.pos;
    livings_data = &_livings_data;
    lt_routine.livings_data = livings_data;
    FilterByIdsAndDistances(pos, _livings_data.enemies, filtered_livings, IDS, 1600.0F);
    FilterByIdAndDistance(pos, filtered_livings, aatxe_livings, GW::Constants::ModelID::UW::BladedAatxe);
    FilterByIdAndDistance(pos, filtered_livings, nightmare_livings, GW::Constants::ModelID::UW::DyingNightmare);
    FilterByIdAndDistance(pos, filtered_livings, dryder_livings, GW::Constants::ModelID::UW::TerrorwebDryder);
    FilterByIdAndDistance(pos, filtered_livings, horseman_livings, GW::Constants::ModelID::UW::FourHorseman);
    FilterByIdAndDistance(pos, filtered_livings, keeper_livings, GW::Constants::ModelID::UW::KeeperOfSouls);
    FilterByIdAndDistance(pos, filtered_livings, skele_livings, GW::Constants::ModelID::UW::SkeletonOfDhuum1);
    FilterByIdAndDistance(pos, filtered_livings, skele_livings, GW::Constants::ModelID::UW::SkeletonOfDhuum2);
    SortByDistance(player_data, aatxe_livings);
    SortByDistance(player_data, nightmare_livings);
    SortByDistance(player_data, horseman_livings);
    SortByDistance(player_data, keeper_livings);
    SortByDistance(player_data, dryder_livings);
    SortByDistance(player_data, skele_livings);

    if (TankIsSoloLT())
        lt_routine.Update();
}

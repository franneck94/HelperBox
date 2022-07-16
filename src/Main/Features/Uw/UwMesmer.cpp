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
static constexpr auto SPAWN_SPIRIT_ID = uint32_t{2374};
static constexpr auto MAX_TABLE_LENGTH = uint32_t{6U};
static constexpr auto IDS = std::array<uint32_t, 6U>{GW::Constants::ModelID::UW::BladedAatxe,
                                                     GW::Constants::ModelID::UW::DyingNightmare,
                                                     GW::Constants::ModelID::UW::TerrorwebDryder,
                                                     GW::Constants::ModelID::UW::FourHorseman,
                                                     GW::Constants::ModelID::UW::SkeletonOfDhuum1,
                                                     GW::Constants::ModelID::UW::SkeletonOfDhuum2};
} // namespace

bool LtRoutine::EnemyShouldGetEmpathy(const std::vector<GW::AgentLiving *> &enemies, const GW::AgentLiving *enemy)
{
    if (!enemy->GetIsAttacking())
        return false;

    if (enemy->hp < 0.25F)
        return false;

    const auto closest_id = GetClosestToPosition(enemy->pos, enemies, enemy->agent_id);
    if (!closest_id)
        return false;
    const auto other_enemy = GW::Agents::GetAgentByID(closest_id);
    if (!other_enemy)
        return false;
    const auto other_enemy_living = other_enemy->GetAsAgentLiving();
    if (!other_enemy_living)
        return false;

    const auto dist = GW::GetDistance(other_enemy->pos, enemy->pos);
    if (dist < GW::Constants::Range::Adjacent && other_enemy_living->GetIsHexed())
        return false;

    return true;
}

bool LtRoutine::CastHexesOnEnemyType(const std::vector<GW::AgentLiving *> &enemies,
                                     const std::vector<GW::AgentLiving *> &filtered_enemies,
                                     uint32_t &last_skill,
                                     uint32_t &last_id,
                                     const bool use_empathy)
{
    if (filtered_enemies.end() == std::find_if(filtered_enemies.begin(),
                                               filtered_enemies.end(),
                                               [&](const auto &enemy) { return enemy->agent_id == last_id; }))
    {
        last_skill = 0;
        last_id = 0;
    }

    auto max_num_skills = uint32_t{2U};
    if (use_empathy)
        ++max_num_skills;

    for (size_t i = 0; i < filtered_enemies.size(); ++i)
    {
        const auto &enemy = filtered_enemies[i];
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

        const auto target_should_get_empathy = EnemyShouldGetEmpathy(enemies, enemy);

        if (last_skill == 0 && !enemy->GetIsHexed() &&
            RoutineState::FINISHED == skillbar->worry.Cast(player_data->energy, id))
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
                 (!target_should_get_empathy ||
                  RoutineState::FINISHED == skillbar->empathy.Cast(player_data->energy, id)))
        {
            if (target_should_get_empathy)
            {
                if (!player_data->target || (player_data->target && player_data->target->agent_id != id))
                    player_data->ChangeTarget(id);
            }
            ++last_skill;
            last_id = id;
            return true;
        }
    }

    return false;
}

bool LtRoutine::DoNeedEnchNow(const GW::Constants::SkillID ench_id) const
{
    const auto found = player_data->HasEffect(ench_id);
    const auto data = GW::SkillbarMgr::GetSkillConstantData((uint32_t)ench_id);

    const auto duration_s = player_data->GetRemainingEffectDuration(ench_id) / 1000.0F;
    const auto trigger_time_s = data ? data->activation + data->aftercast : 1.0F;

    if (!found || duration_s < trigger_time_s)
        return true;

    return false;
}

bool LtRoutine::DoNeedVisage(const std::vector<GW::AgentLiving *> &enemies,
                             const std::vector<GW::AgentLiving *> &aatxes,
                             const std::vector<GW::AgentLiving *> &graspings) const
{
    const auto closest_id = GetClosestToPosition(player_data->pos, enemies, 0);
    const auto closest_enemy = GW::Agents::GetAgentByID(closest_id);
    if (!closest_enemy)
        return false;

    const auto closest_enemy_living = closest_enemy->GetAsAgentLiving();
    if (!closest_enemy_living)
        return false;

    const auto enemies_in_range = (aatxes.size() || graspings.size());
    const auto spike_has_begun = closest_enemy_living->hp > 0.80F;

    return (enemies_in_range && spike_has_begun);
}

bool LtRoutine::RoutineSelfEnches(const std::vector<GW::AgentLiving *> &enemies) const
{
    const auto nightmares = FilterById(enemies, GW::Constants::ModelID::UW::DyingNightmare);
    const auto dryders = FilterById(enemies, GW::Constants::ModelID::UW::TerrorwebDryder);
    const auto aatxes = FilterById(enemies, GW::Constants::ModelID::UW::BladedAatxe);
    const auto graspings = FilterById(enemies, GW::Constants::ModelID::UW::GraspingDarkness);

    const auto need_obsi = (nightmares.size() || dryders.size());
    const auto need_stoneflesh = (aatxes.size() || graspings.size());
    const auto need_mantra = (aatxes.size() || graspings.size());
    const auto need_visage = DoNeedVisage(enemies, aatxes, graspings);

    if (need_obsi && DoNeedEnchNow(GW::Constants::SkillID::Obsidian_Flesh) &&
        (RoutineState::FINISHED == skillbar->obsi.Cast(player_data->energy)))
        return true;

    if (need_stoneflesh && DoNeedEnchNow(GW::Constants::SkillID::Stoneflesh_Aura) &&
        (RoutineState::FINISHED == skillbar->stoneflesh.Cast(player_data->energy)))
        return true;

    if (need_mantra && DoNeedEnchNow(GW::Constants::SkillID::Mantra_of_Resolve) &&
        (RoutineState::FINISHED == skillbar->mantra_of_resolve.Cast(player_data->energy)))
        return true;

    if (need_visage && DoNeedEnchNow(GW::Constants::SkillID::Sympathetic_Visage) &&
        (RoutineState::FINISHED == skillbar->visage.Cast(player_data->energy)))
        return true;

    return false;
}

bool LtRoutine::RoutineSpikeBall(const std::vector<GW::AgentLiving *> &enemies, const auto include_graspings)
{
    const auto nightmares = FilterById(enemies, GW::Constants::ModelID::UW::DyingNightmare);
    const auto aatxes = FilterById(enemies, GW::Constants::ModelID::UW::BladedAatxe);
    const auto dryders = FilterById(enemies, GW::Constants::ModelID::UW::TerrorwebDryder);
    const auto graspings = FilterById(enemies, GW::Constants::ModelID::UW::GraspingDarkness);

    if (CastHexesOnEnemyType(enemies, nightmares, last_nightmare_skill, last_nightmare_id, true))
        return true;
    if (CastHexesOnEnemyType(enemies, dryders, last_dryder_skill, last_dryder_id, true))
        return true;
    if (dryders.size() == 0)
    {
        if (CastHexesOnEnemyType(enemies, aatxes, last_aatxe_skill, last_aatxe_id, false))
            return true;
        if (include_graspings &&
            CastHexesOnEnemyType(enemies, graspings, last_graspings_skill, last_graspings_id, false))
            return true;
    }

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
    static auto gone_to_npc = false;
    static auto took_quest = false;

    if (!IsUw())
        return RoutineState::FINISHED;

    delay_ms = 250L;
    if (gone_to_npc)
        delay_ms = long{500L};

    if (!ActionABC::HasWaitedLongEnough(delay_ms)) // ms
        return RoutineState::ACTIVE;

    if (starting_active)
    {
        const auto agent_id = GetClosestNpcbyId(*player_data, livings_data->npcs, SPAWN_SPIRIT_ID);
        if (!agent_id)
        {
            starting_active = false;
            gone_to_npc = false;
            took_quest = false;
            return RoutineState::FINISHED;
        }

        if (!player_data->target)
        {
            if (agent_id)
            {
                player_data->ChangeTarget(agent_id);
                if (RoutineState::FINISHED == skillbar->ebon.Cast(player_data->energy, agent_id))
                    return RoutineState::FINISHED;
            }
        }
        else
        {
            if (!gone_to_npc)
            {
                const auto agent = GW::Agents::GetAgentByID(agent_id);
                GW::Agents::GoNPC(agent, 0U);
                gone_to_npc = true;
                took_quest = false;
                return RoutineState::ACTIVE;
            }
            if (gone_to_npc && !took_quest)
            {
                TakeChamber();
                took_quest = true;
                return RoutineState::ACTIVE;
            }
            TakeChamber();

            if (skillbar->stoneflesh.CanBeCasted(player_data->energy) &&
                RoutineState::FINISHED == skillbar->stoneflesh.Cast(player_data->energy))
                return RoutineState::FINISHED;
            if (skillbar->mantra_of_resolve.CanBeCasted(player_data->energy) &&
                RoutineState::FINISHED == skillbar->mantra_of_resolve.Cast(player_data->energy))
                return RoutineState::FINISHED;
            if (skillbar->visage.CanBeCasted(player_data->energy) &&
                RoutineState::FINISHED == skillbar->visage.Cast(player_data->energy))
                return RoutineState::FINISHED;
            if (skillbar->obsi.CanBeCasted(player_data->energy) &&
                RoutineState::FINISHED == skillbar->obsi.Cast(player_data->energy))
                return RoutineState::FINISHED;

            starting_active = false;
            gone_to_npc = false;
            took_quest = false;
            action_state = ActionState::INACTIVE;
        }
    }

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

    if (IsAOnSpawnPlateau(player_data->pos) && GW::PartyMgr::GetPartySize() <= 6 && !player_data->target &&
        load_cb_triggered)
    {
        starting_active = true;
        action_state = ActionState::ACTIVE;
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

void UwMesmer::Draw()
{
    if (!visible)
        return;

    if (!UwHelperActivationConditions(false))
        return;
    if (!IsUwMesmer(player_data))
        return;

    ImGui::SetNextWindowSize(ImVec2(200.0F, 240.0F), ImGuiCond_FirstUseEver);

    if (ImGui::Begin(Name(), nullptr, GetWinFlags()))
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

#ifdef _DEBUG
    if (TankIsSoloLT())
    {
        if (ImGui::Begin("LtWindow", nullptr))
            lt_routine.Draw();
        ImGui::End();
    }
#endif
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

    if (!player_data.ValidateData(UwHelperActivationConditions, false))
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
    lt_routine.load_cb_triggered = UwMetadata::Instance().load_cb_triggered;

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

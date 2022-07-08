#include <cmath>
#include <cstdint>
#include <vector>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/Context/GameContext.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Party.h>
#include <GWCA/GameEntities/Player.h>
#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Packets/Opcodes.h>

#include <ActionsUw.h>
#include <Base/HelperBox.h>
#include <DataPlayer.h>
#include <DataSkillbar.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <HelperAgents.h>
#include <HelperUw.h>
#include <HelperUwPos.h>
#include <Logger.h>
#include <MathUtils.h>
#include <Timer.h>
#include <Types.h>

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

#include "UwDhuumBitch.h"

namespace
{
static auto move_ongoing = false;
static ActionState *damage_action_state = nullptr;
static auto lt_is_ready = false;
}; // namespace

UwDhuumBitch::UwDhuumBitch() : UwHelperABC(), skillbar({}), db_routinme(&player_data, &skillbar, livings_data)
{
    if (skillbar.ValidateData())
        skillbar.Load();
};

void UwDhuumBitch::Draw(IDirect3DDevice9 *)
{
    if (!visible || !player_data.ValidateData(UwHelperActivationConditions) || !IsDhuumBitch(player_data))
        return;

    ImGui::SetNextWindowSize(ImVec2(110.0F, 170.0F), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("UwDhuumBitch", nullptr, GetWinFlags()))
    {
        db_routinme.Draw();
        DrawMovingButtons(moves, move_ongoing, move_idx);
    }
    ImGui::End();

#ifdef _DEBUG
    if (IsUw() && show_debug_map && livings_data)
        DrawMap(player_data.pos, livings_data->enemies, moves[move_idx]->pos, "DbMap");
#endif
}

void UwDhuumBitch::UpdateUw()
{
    UpdateUwEntry();
    MoveABC::UpdatedUwMoves(player_data, livings_data, moves, move_idx, move_ongoing);

    if (num_finished_objectives == 10U && !move_ongoing && moves[move_idx]->name == "Go To Dhuum 1")
    {
        moves[move_idx]->Execute();
        move_ongoing = true;
    }

    const auto is_hm_trigger_move = (moves[move_idx]->name == "Talk Lab" || moves[move_idx]->name == "Go To Dhuum 1");
    if (lt_is_ready && !move_ongoing && is_hm_trigger_move)
    {
        moves[move_idx]->Execute();
        move_ongoing = true;
        lt_is_ready = false;
    }
    if (move_ongoing)
        lt_is_ready = false;
}

void UwDhuumBitch::UpdateUwEntry()
{
    if (TankIsFullteamLT())
        load_cb_triggered = false;

    if (load_cb_triggered)
    {
        move_idx = 0;
        move_ongoing = false;
        moves[0]->Execute();
        load_cb_triggered = false;
        move_ongoing = true;

        *damage_action_state = ActionState::ACTIVE;
    }
}

void UwDhuumBitch::Update(float, const AgentLivingData &_livings_data)
{
    if (!player_data.ValidateData(UwHelperActivationConditions))
    {
        move_idx = 0;
        move_ongoing = false;
        db_routinme.action_state = ActionState::INACTIVE;
        return;
    }
    player_data.Update();
    livings_data = &_livings_data;
    db_routinme.livings_data = livings_data;

    if (!IsDhuumBitch(player_data))
        return;

    if (IsUw() && first_frame)
    {
        UpdateUwInfo(player_data, moves, move_idx, true, move_ongoing);
        first_frame = false;
    }

    if (!skillbar.ValidateData())
        return;
    skillbar.Update();

    damage_action_state = &db_routinme.action_state;

    if (IsUw())
    {
        UpdateUwInfo(player_data, moves, move_idx, false, move_ongoing);
        UpdateUw();
    }

    db_routinme.Update();
}

DbRoutine::DbRoutine(DataPlayer *p, DbSkillbarData *s, const AgentLivingData *a)
    : DbActionABC(p, "DbRoutine", s), livings_data(a)
{
}

bool DbRoutine::CastPiOnTarget() const
{
    if (!player_data->target)
        return false;

    const auto target_living = player_data->target->GetAsAgentLiving();
    if (!target_living || target_living->allegiance != GW::Constants::Allegiance::Enemy)
        return false;

    const auto dist = GW::GetDistance(player_data->pos, target_living->pos);
    if (dist > GW::Constants::Range::Spellcast)
        return false;

    if (RoutineState::FINISHED == skillbar->pi.Cast(player_data->energy, target_living->agent_id))
        return true;

    return false;
}

bool DbRoutine::RoutineKillSkele() const
{
    if (RoutineState::FINISHED == skillbar->sos.Cast(player_data->energy) || CastPiOnTarget())
        return true;

    return false;
}

bool DbRoutine::RoutineKillEnemiesStandard() const
{
    const auto found_honor = player_data->HasEffect(GW::Constants::SkillID::Ebon_Battle_Standard_of_Honor);

    if (!found_honor && RoutineState::FINISHED == skillbar->honor.Cast(player_data->energy))
        return true;

    if (RoutineState::FINISHED == skillbar->sos.Cast(player_data->energy) ||
        RoutineState::FINISHED == skillbar->vamp.Cast(player_data->energy))
        return true;

    return false;
}

bool DbRoutine::RoutineValeSpirits() const
{
    const auto found_honor = player_data->HasEffect(GW::Constants::SkillID::Ebon_Battle_Standard_of_Honor);
    const auto found_eoe = player_data->HasEffect(GW::Constants::SkillID::Edge_of_Extinction);
    const auto found_winnow = player_data->HasEffect(GW::Constants::SkillID::Winnowing);

    if (!found_honor && RoutineState::FINISHED == skillbar->honor.Cast(player_data->energy))
        return true;

    if (RoutineState::FINISHED == skillbar->sos.Cast(player_data->energy) ||
        RoutineState::FINISHED == skillbar->vamp.Cast(player_data->energy))
        return true;

    if (!found_eoe && player_data->energy >= 30U && RoutineState::FINISHED == skillbar->eoe.Cast(player_data->energy))
        return true;

    if (!found_winnow && RoutineState::FINISHED == skillbar->winnow.Cast(player_data->energy))
        return true;

    return false;
}

bool DbRoutine::RoutineDhuumRecharge() const
{
    static auto qz_timer = clock();

    const auto found_qz = player_data->HasEffect(GW::Constants::SkillID::Quickening_Zephyr);

    const auto qz_diff_ms = TIMER_DIFF(qz_timer);
    if (qz_diff_ms > 36'000 || !found_qz)
    {
        if (!found_qz && RoutineState::FINISHED == skillbar->sq.Cast(player_data->energy))
            return true;

        if (RoutineState::FINISHED == skillbar->qz.Cast(player_data->energy))
        {
            qz_timer = clock();
            return true;
        }
    }

    return false;
}

bool DbRoutine::RoutineDhuumDamage() const
{
    const auto found_honor = player_data->HasEffect(GW::Constants::SkillID::Ebon_Battle_Standard_of_Honor);
    const auto found_winnow = player_data->HasEffect(GW::Constants::SkillID::Winnowing);

    if (!found_honor && player_data->energy > 33U &&
        RoutineState::FINISHED == skillbar->honor.Cast(player_data->energy))
        return true;

    if (!found_winnow && RoutineState::FINISHED == skillbar->winnow.Cast(player_data->energy))
        return true;

    if (RoutineState::FINISHED == skillbar->sos.Cast(player_data->energy))
        return true;

    return false;
}

RoutineState DbRoutine::Routine()
{
    static auto dhuum_fight_ongoing = false;
    const auto is_in_dhuum_room = IsInDhuumRoom(player_data->pos);

    if (!IsUw())
        return RoutineState::FINISHED;

    if (!player_data->CanCast() || !livings_data)
        return RoutineState::ACTIVE;

    if (!ActionABC::HasWaitedLongEnough())
        return RoutineState::ACTIVE;

    if (TankIsFullteamLT() && !is_in_dhuum_room)
        return RoutineState::FINISHED;

    if (IsAtChamberSkele(player_data->pos) || IsAtBasementSkele(player_data->pos) ||
        IsRightAtValeHouse(player_data->pos))
    {
        const auto enemies = FilterAgentsByRange(livings_data->enemies, *player_data, GW::Constants::Range::Earshot);
        if (enemies.size() == 0)
            return RoutineState::ACTIVE;

        if (!player_data->living->GetIsAttacking() && player_data->CanAttack())
            TargetAndAttackEnemyInAggro(*player_data, livings_data->enemies, GW::Constants::Range::Earshot);

        if (RoutineKillSkele())
            return RoutineState::FINISHED;
    }

    // If mindblades were not stucked by LT, or back patrol aggro
    if (IsAtFusePulls(player_data->pos) || InBackPatrolArea(player_data->pos))
    {
        const auto enemies = FilterAgentsByRange(livings_data->enemies, *player_data, GW::Constants::Range::Earshot);
        if (enemies.size() == 0)
            return RoutineState::ACTIVE;
        RoutineKillEnemiesStandard();
    }

    if (IsAtValeSpirits(player_data->pos))
    {
        const auto enemies = FilterAgentsByRange(livings_data->enemies, *player_data, 1700.0F);
        if (enemies.size() == 0)
            return RoutineState::ACTIVE;

        if (!player_data->living->GetIsAttacking() && player_data->CanAttack())
            TargetAndAttackEnemyInAggro(*player_data, livings_data->enemies, 1700.0F);

        if (RoutineValeSpirits())
            return RoutineState::FINISHED;
    }

    if (!is_in_dhuum_room)
        return RoutineState::FINISHED;

    if (dhuum_fight_ongoing && RoutineDhuumRecharge())
        return RoutineState::FINISHED;

    auto dhuum_id = uint32_t{0};
    auto dhuum_hp = float{1.0F};
    const auto is_in_dhuum_fight = IsInDhuumFight(&dhuum_id, &dhuum_hp);
    if (!is_in_dhuum_fight && dhuum_fight_ongoing)
        dhuum_fight_ongoing = false;

    if (!is_in_dhuum_fight && DhuumFightDone(livings_data->npcs))
    {
        action_state = ActionState::INACTIVE;
        move_ongoing = false;
    }

    if (!is_in_dhuum_fight || !dhuum_id)
        return RoutineState::FINISHED;
    dhuum_fight_ongoing = true;

    const auto dhuum_agent = GW::Agents::GetAgentByID(dhuum_id);
    if (!dhuum_agent)
        return RoutineState::FINISHED;
    const auto dhuum_dist = GW::GetDistance(player_data->pos, dhuum_agent->pos);

    if (dhuum_hp < 0.20F)
        return RoutineState::FINISHED;

    if (dhuum_dist > GW::Constants::Range::Spellcast)
        return RoutineState::FINISHED;

    if (DhuumIsCastingJudgement(dhuum_id) &&
        (RoutineState::FINISHED == skillbar->pi.Cast(player_data->energy, dhuum_id)))
        return RoutineState::FINISHED;

    if (RoutineDhuumDamage())
        return RoutineState::FINISHED;

    if (!player_data->living->GetIsAttacking() && player_data->CanAttack())
        TargetAndAttackEnemyInAggro(*player_data, livings_data->enemies, GW::Constants::Range::Earshot);

    return RoutineState::FINISHED;
}

bool DbRoutine::PauseRoutine()
{
    if (player_data->living->GetIsMoving())
        return true;

    if (player_data->target)
    {
        if (TargetIsReaper(*player_data) && (GW::GetDistance(player_data->pos, player_data->target->pos) < 300.0F))
            return true;
    }

    return false;
}

void DbRoutine::Update()
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
    }

    if (action_state == ActionState::ACTIVE)
        (void)Routine();
}

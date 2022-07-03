#include "stdafx.h"

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

#include <HelperBox.h>
#include <Logger.h>
#include <Timer.h>

#include <Actions.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <MathUtils.h>
#include <PlayerData.h>
#include <SkillbarData.h>
#include <Types.h>
#include <UwHelper.h>

#include "DbWindow.h"

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

namespace
{
static auto move_ongoing = false;
static ActionState *damage_action_state = nullptr;
}; // namespace

DbWindow::DbWindow() : player_data({}), skillbar({}), damage(&player_data, &skillbar)
{
    if (skillbar.ValidateData())
        skillbar.Load();

    GW::StoC::RegisterPacketCallback<GW::Packet::StoC::MapLoaded>(
        &MapLoaded_Entry,
        [this](GW::HookStatus *status, GW::Packet::StoC::MapLoaded *packet) -> void {
            load_cb_triggered = ExplorableLoadCallback(status, packet);
        });

    GW::StoC::RegisterPacketCallback<GW::Packet::StoC::GenericValue>(
        &GenericValue_Entry,
        [this](GW::HookStatus *, GW::Packet::StoC::GenericValue *packet) -> void {
            if (move_ongoing && player_data.SkillStoppedCallback(packet))
                interrupted = true;
        });
};

void DbWindow::Draw(IDirect3DDevice9 *)
{
    if (!visible)
        return;

    if (!player_data.ValidateData(UwHelperActivationConditions))
        return;
    if (!IsDhuumBitch(player_data))
        return;

    ImGui::SetNextWindowSize(ImVec2(110.0F, 170.0F), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("DbWindow", nullptr, GetWinFlags()))
    {
        damage.Draw();
        DrawMovingButtons(moves, move_ongoing, move_idx);
    }
    ImGui::End();

#ifdef _DEBUG
    if (IsUw() && show_debug_map)
        DrawMap(player_data.pos, moves, move_idx, "DbMap");
#endif
}

void DbWindow::UpdateUw()
{
    UpdateUwEntry();
    UpdatedUwMoves_Main(player_data, moves, move_idx, move_ongoing);
}

void DbWindow::UpdateUwEntry()
{
    if (load_cb_triggered)
    {
        move_idx = 0;
        moves[0].Execute();
        load_cb_triggered = false;
        move_ongoing = true;

        *damage_action_state = ActionState::ACTIVE;
    }
}

void DbWindow::Update(float)
{
    if (!player_data.ValidateData(UwHelperActivationConditions))
        return;
    player_data.Update();

    if (!IsDhuumBitch(player_data))
        return;

    if (IsUw() && first_frame)
    {
        UpdateUwInfo(player_data, moves, move_idx, true);
        first_frame = false;
    }

    if (IsLoading() || IsOutpost())
        move_idx = 0;

    if (!skillbar.ValidateData())
        return;
    skillbar.Update();

    damage_action_state = &damage.action_state;

    if (IsUw())
    {
        UpdateUwInfo(player_data, moves, move_idx, false);
        UpdateUw();
    }

    damage.Update();
}

Damage::Damage(PlayerData *p, DbSkillbarData *s) : DbActionABC(p, "Damage", s)
{
}

bool Damage::CastPiOnTarget() const
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

bool Damage::RoutineKillSkele() const
{
    if (RoutineState::FINISHED == skillbar->sos.Cast(player_data->energy))
        return true;

    if (!player_data->target)
        return false;

    if (CastPiOnTarget())
        return true;

    return false;
}

bool Damage::RoutineValeSpirits() const
{
    const auto found_honor = player_data->HasEffect(GW::Constants::SkillID::Ebon_Battle_Standard_of_Honor);
    const auto found_eoe = player_data->HasEffect(GW::Constants::SkillID::Edge_of_Extinction);
    const auto found_winnow = player_data->HasEffect(GW::Constants::SkillID::Winnowing);

    if (!found_honor && RoutineState::FINISHED == skillbar->honor.Cast(player_data->energy))
        return true;

    if (RoutineState::FINISHED == skillbar->sos.Cast(player_data->energy))
        return true;

    if (RoutineState::FINISHED == skillbar->vamp.Cast(player_data->energy))
        return true;

    if (!found_eoe && RoutineState::FINISHED == skillbar->eoe.Cast(player_data->energy))
        return true;

    if (!found_winnow && RoutineState::FINISHED == skillbar->winnow.Cast(player_data->energy))
        return true;

    return false;
}

bool Damage::RoutineDhuumRecharge() const
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

bool Damage::RoutineDhuumDamage() const
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

RoutineState Damage::Routine()
{
    static auto dhuum_fight_ongoing = false;

    if (!IsUw())
        return RoutineState::FINISHED;

    if (!player_data->CanCast())
        return RoutineState::ACTIVE;

    if (!ActionABC::HasWaitedLongEnough())
        return RoutineState::ACTIVE;

    if (IsAtChamberSkele(player_data->pos) || IsAtBasementSkele(player_data->pos) ||
        IsRightAtValeHouse(player_data->pos))
    {
        const auto enemies = GetEnemiesInRange(*player_data, GW::Constants::Range::Earshot);
        if (enemies.size() == 0)
            return RoutineState::ACTIVE;

        if (!player_data->living->GetIsAttacking() && player_data->CanAttack())
            TargetAndAttackEnemyInAggro(*player_data, GW::Constants::Range::Earshot);

        if (RoutineKillSkele())
            return RoutineState::FINISHED;
    }

    if (IsAtValeSpirits(player_data->pos))
    {
        const auto enemies = GetEnemiesInRange(*player_data, 1700.0F);
        if (enemies.size() == 0)
            return RoutineState::ACTIVE;

        if (!player_data->living->GetIsAttacking() && player_data->CanAttack())
            TargetAndAttackEnemyInAggro(*player_data, 1700.0F);

        if (RoutineValeSpirits())
            return RoutineState::FINISHED;
    }

    const auto is_in_dhuum_room = IsInDhuumRoom(player_data->pos);
    if (!is_in_dhuum_room)
        return RoutineState::FINISHED;

    if (dhuum_fight_ongoing && RoutineDhuumRecharge())
        return RoutineState::FINISHED;

    auto dhuum_id = uint32_t{0};
    auto dhuum_hp = float{1.0F};
    const auto currently_in_dhuum_fight = IsInDhuumFight(&dhuum_id, &dhuum_hp);
    if (!currently_in_dhuum_fight && dhuum_fight_ongoing)
        dhuum_fight_ongoing = false;

    if (!currently_in_dhuum_fight || !dhuum_id)
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
        TargetAndAttackEnemyInAggro(*player_data, GW::Constants::Range::Earshot);

    return RoutineState::FINISHED;
}

bool Damage::PauseRoutine()
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

void Damage::Update()
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

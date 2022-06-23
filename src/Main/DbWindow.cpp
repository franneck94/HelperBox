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

#include <GuiUtils.h>
#include <HelperBox.h>
#include <Logger.h>
#include <Timer.h>

#include <Actions.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <MathUtils.h>
#include <Player.h>
#include <SafeFuncs.h>
#include <Skillbars.h>
#include <Types.h>
#include <UwHelper.h>

#include "DbWindow.h"

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

namespace
{
static auto move_ongoing = false;
}; // namespace

DbWindow::DbWindow() : player({}), skillbar({}), damage(&player, &skillbar)
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
        [this](GW::HookStatus *status, GW::Packet::StoC::GenericValue *packet) -> void {
            UNREFERENCED_PARAMETER(status);
            if (move_ongoing && player.SkillStoppedCallback(packet))
                interrupted = true;
        });
};

void DbWindow::Draw(IDirect3DDevice9 *pDevice)
{
    UNREFERENCED_PARAMETER(pDevice);

    if (!visible)
        return;

    if (!ActivationConditions())
        return;

    if (IsLoading())
        return;

    if ((!IsUw() && !IsUwEntryOutpost()))
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
        DrawMap(player, moves, move_idx, "DbMap");
#endif
}

void DbWindow::UpdateUw()
{
    UpdateUwEntry();
    UpdateUwMoves();
}

void DbWindow::UpdateUwEntry()
{
    if (load_cb_triggered)
    {
        moves[0].Execute();
        load_cb_triggered = false;
        move_ongoing = true;
    }
}

void DbWindow::UpdateUwMoves()
{
    UpdatedUwMoves_Main(player, moves, move_idx, move_ongoing);
}

void DbWindow::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);
    static auto last_pos = player.pos;

    if (!player.ValidateData())
        return;
    player.Update();

    if (!ActivationConditions())
        return;

    if (IsLoading() || IsOutpost())
        move_idx = 0;

    if (!skillbar.ValidateData())
        return;
    skillbar.Update();

    if (IsUw())
    {
        UpdateUw();

        const auto curr_pos = player.pos;
        const auto dist = GW::GetDistance(last_pos, curr_pos);
        if (dist > 5'000.0F)
        {
            Log::Info("Ported!");
            move_idx = GetClostestMove(player, moves);
        }
        last_pos = curr_pos;
    }

    damage.Update();
}

bool DbWindow::ActivationConditions() const
{
    if (!GW::Map::GetIsMapLoaded())
        return false;

    if (!GW::PartyMgr::GetIsPartyLoaded())
        return false;

    if (!IsDhuumBitch(player))
        return false;

    if (IsUwEntryOutpost() || IsUw())
        return true;

    return false;
}

Damage::Damage(Player *p, DbSkillbar *s) : DbActionABC(p, "Damage", s)
{
}

bool Damage::CastPiOnTarget() const
{
    if (!player->target)
        return false;

    const auto target_living = player->target->GetAsAgentLiving();
    if (!target_living || target_living->allegiance != static_cast<uint8_t>(GW::Constants::Allegiance::Enemy))
        return false;

    const auto dist = GW::GetDistance(player->pos, target_living->pos);
    if (dist > GW::Constants::Range::Spellcast)
        return false;

    if (RoutineState::FINISHED == skillbar->pi.Cast(player->energy, target_living->agent_id))
        return true;

    return false;
}

bool Damage::RoutineAtChamberSkele() const
{
    if (RoutineState::FINISHED == skillbar->sos.Cast(player->energy))
        return true;

    if (!player->target)
        return false;

    if (CastPiOnTarget())
        return true;

    return false;
}

bool Damage::RoutineValeSpirits() const
{
    const auto found_honor = player->HasEffect(GW::Constants::SkillID::Ebon_Battle_Standard_of_Honor);
    const auto found_eoe = player->HasEffect(GW::Constants::SkillID::Edge_of_Extinction);
    const auto found_winnow = player->HasEffect(GW::Constants::SkillID::Winnowing);

    if (!found_honor)
    {
        if (RoutineState::FINISHED == skillbar->honor.Cast(player->energy))
            return true;
    }

    if (RoutineState::FINISHED == skillbar->sos.Cast(player->energy))
        return true;

    if (!found_eoe)
    {
        if (RoutineState::FINISHED == skillbar->eoe.Cast(player->energy))
            return true;
    }

    if (!found_winnow)
    {
        if (RoutineState::FINISHED == skillbar->winnow.Cast(player->energy))
            return true;
    }

    return false;
}

bool Damage::RoutinePI(const uint32_t dhuum_id) const
{
    const auto &skill = skillbar->pi;

    if (!skill.SkillFound())
        return false;

    const auto dhuum_agent = GW::Agents::GetAgentByID(dhuum_id);
    if (!dhuum_agent)
        return false;

    const auto dhuum_living = dhuum_agent->GetAsAgentLiving();
    if (!dhuum_living)
        return false;

    if (dhuum_living->GetIsCasting() && dhuum_living->skill == static_cast<uint32_t>(3085))
        return (RoutineState::FINISHED == skill.Cast(player->energy, dhuum_id));

    return false;
}

bool Damage::RoutineDhuumRecharge() const
{
    static auto qz_timer = clock();

    const auto found_qz = player->HasEffect(GW::Constants::SkillID::Quickening_Zephyr);

    const auto qz_diff_ms = TIMER_DIFF(qz_timer);
    if (qz_diff_ms > 36'000 || !found_qz)
    {
        if (!found_qz && RoutineState::FINISHED == skillbar->sq.Cast(player->energy))
            return true;

        if (RoutineState::FINISHED == skillbar->qz.Cast(player->energy))
        {
            qz_timer = clock();
            return true;
        }
    }

    return false;
}

bool Damage::RoutineDhuumDamage() const
{
    const auto found_honor = player->HasEffect(GW::Constants::SkillID::Ebon_Battle_Standard_of_Honor);
    const auto found_winnow = player->HasEffect(GW::Constants::SkillID::Winnowing);

    if (!found_honor && player->energy > 20U && RoutineState::FINISHED == skillbar->honor.Cast(player->energy))
        return true;

    if (!found_winnow && RoutineState::FINISHED == skillbar->winnow.Cast(player->energy))
        return true;

    if (RoutineState::FINISHED == skillbar->sos.Cast(player->energy))
        return true;

    return false;
}

RoutineState Damage::Routine()
{
    static auto was_in_dhuum_fight = false;

    if (!player->CanCast() && !IsUw())
        return RoutineState::FINISHED;

    if (IsAtChamberSkele(player))
    {
        if (RoutineAtChamberSkele())
            return RoutineState::FINISHED;

        if (!player->living->GetIsAttacking() && player->CanAttack())
            TargetAndAttackEnemyInAggro(*player);
    }

    if (IsInVale(player))
    {
        const auto at_vale_house = IsAtValeHouse(player);
        const auto livings = GetEnemiesInAggro(*player);
        const auto enemies_at_vale_house = livings.size() != 0;
        if (at_vale_house && !enemies_at_vale_house)
            return RoutineState::FINISHED;

        if (CastPiOnTarget())
            return RoutineState::FINISHED;

        if (RoutineValeSpirits())
            return RoutineState::FINISHED;

        if (!player->living->GetIsAttacking() && player->CanAttack())
            TargetAndAttackEnemyInAggro(*player);
    }

    const auto is_in_dhuum_room = IsInDhuumRoom(player);
    if (!is_in_dhuum_room)
        return RoutineState::FINISHED;

    auto dhuum_id = uint32_t{0};
    auto dhuum_hp = float{1.0F};
    const auto is_in_dhuum_fight = IsInDhuumFight(&dhuum_id, &dhuum_hp);

    if (!is_in_dhuum_fight || !dhuum_id || dhuum_hp == 1.0F)
        return RoutineState::FINISHED;
    was_in_dhuum_fight = true;

    const auto dhuum_agent = GW::Agents::GetAgentByID(dhuum_id);
    if (!dhuum_agent)
        return RoutineState::FINISHED;
    const auto dhuum_dist = GW::GetDistance(player->pos, dhuum_agent->pos);

    if (RoutineDhuumRecharge())
        return RoutineState::FINISHED;

    if (dhuum_hp < 0.20F)
        return RoutineState::FINISHED;

    if (dhuum_dist > GW::Constants::Range::Spellcast)
        return RoutineState::FINISHED;

    if (RoutinePI(dhuum_id))
        return RoutineState::FINISHED;

    if (RoutineDhuumDamage())
        return RoutineState::FINISHED;

    return RoutineState::FINISHED;
}

bool Damage::PauseRoutine()
{
    if (player->target)
    {
        const auto dist = GW::GetDistance(player->pos, player->target->pos);

        if (TargetIsReaper(*player) && (dist < 300.0F))
            return true;
    }

    if (player->living->GetIsMoving())
        return true;

    return false;
}

bool Damage::ResumeRoutine()
{
    return !PauseRoutine();
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

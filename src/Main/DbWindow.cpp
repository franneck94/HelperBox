#include "stdafx.h"

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

namespace
{
static ActionState *emo_casting_action_state = nullptr;
static auto send_move = false;
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
};

void DbWindow::Draw(IDirect3DDevice9 *pDevice)
{
    UNREFERENCED_PARAMETER(pDevice);

    if (!visible)
        return;

    if (!ActivationConditions())
        return;

    ImGui::SetNextWindowSize(ImVec2(110.0F, 170.0F), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("DbWindow", nullptr, GetWinFlags()))
    {
        damage.Draw();

        if (IsUw() || IsUwEntryOutpost())
        {
            DrawMovingButtons(moves, send_move, move_idx);
        }
    }

    ImGui::End();
}

void DbWindow::UpdateUw()
{
    UpdateUwEntry();
    UpdateUwMoves();
}

void DbWindow::UpdateUwEntry()
{
    // if (load_cb_triggered)
    // {
    //     moves[0].Execute();
    //     load_cb_triggered = false;
    //     send_move = true;
    // }
}

void DbWindow::UpdateUwMoves()
{
    if (!send_move)
        return;

    if ((move_idx >= moves.size() - 1U) || !GamePosCompare(player.pos, moves[move_idx].pos, 0.001F))
        return;

    const auto ret =
        Move::UpdateMove(player, send_move, moves[move_idx], moves[move_idx + 1U], moves[move_idx].wait_aggro_range);

    if (ret)
        ++move_idx;
}

void DbWindow::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);

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

    emo_casting_action_state = &damage.action_state;

    if (IsUw())
        UpdateUw();

    damage.Update();
}

bool DbWindow::ActivationConditions() const
{
    if (!GW::PartyMgr::GetIsPartyLoaded())
        return false;

    if (player.secondary == GW::Constants::Profession::Ranger)
        return true;

    return false;
}

Damage::Damage(Player *p, DbSkillbar *s) : DbActionABC(p, "Damage", s)
{
}

RoutineState Damage::RoutineValeSpirits() const
{
    const auto found_honor = player->HasEffect(GW::Constants::SkillID::Ebon_Battle_Standard_of_Honor);
    const auto found_eoe = player->HasEffect(GW::Constants::SkillID::Edge_of_Extinction);
    const auto found_winnow = player->HasEffect(GW::Constants::SkillID::Winnowing);

    if (!found_honor)
    {
        const auto honor_state = skillbar->honor.Cast(player->energy);
        if (honor_state == RoutineState::FINISHED)
            return RoutineState::FINISHED;
    }

    if (!found_eoe)
    {
        const auto eoe_state = skillbar->eoe.Cast(player->energy);
        if (eoe_state == RoutineState::FINISHED)
            return RoutineState::FINISHED;
    }

    if (!found_winnow)
    {
        const auto winnow_state = skillbar->winnow.Cast(player->energy);
        if (winnow_state == RoutineState::FINISHED)
            return RoutineState::FINISHED;
    }

    const auto sos_state = skillbar->sos.Cast(player->energy);
    if (sos_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    return RoutineState::ACTIVE;
}

RoutineState Damage::RoutinePI(const uint32_t dhuum_id) const
{
    const auto &skill = skillbar->pi;

    if (!skill.SkillFound())
        return RoutineState::ACTIVE;

    const auto dhuum_agent = GW::Agents::GetAgentByID(dhuum_id);
    if (!dhuum_agent)
        return RoutineState::ACTIVE;

    const auto dhuum_living = dhuum_agent->GetAsAgentLiving();
    if (!dhuum_living)
        return RoutineState::ACTIVE;

    if (dhuum_living->GetIsCasting() && dhuum_living->skill == static_cast<uint32_t>(3085))
        return skillbar->pi.Cast(player->energy, dhuum_id);

    return RoutineState::ACTIVE;
}

RoutineState Damage::RoutineDhuum() const
{
    const auto found_honor = player->HasEffect(GW::Constants::SkillID::Ebon_Battle_Standard_of_Honor);
    const auto found_eoe = player->HasEffect(GW::Constants::SkillID::Edge_of_Extinction);
    const auto found_qz = player->HasEffect(GW::Constants::SkillID::Quickening_Zephyr); // every 36s cast

    if (!found_honor)
    {
        const auto honor_state = skillbar->honor.Cast(player->energy);
        if (honor_state == RoutineState::FINISHED)
            return RoutineState::FINISHED;
    }

    if (!found_eoe)
    {
        const auto eoe_state = skillbar->eoe.Cast(player->energy);
        if (eoe_state == RoutineState::FINISHED)
            return RoutineState::FINISHED;
    }

    if (!found_qz)
    {
        const auto sq_state = skillbar->sq.Cast(player->energy);
        if (sq_state == RoutineState::FINISHED)
            return RoutineState::FINISHED;

        const auto qz_state = skillbar->qz.Cast(player->energy);
        if (qz_state == RoutineState::FINISHED)
            return RoutineState::FINISHED;
    }

    const auto sos_state = skillbar->sos.Cast(player->energy);
    if (sos_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    return RoutineState::ACTIVE;
}

RoutineState Damage::Routine()
{
    static auto was_in_dhuum_fight = false;

    if (!player->CanCast())
        return RoutineState::FINISHED;

    if (!IsUw())
        return RoutineState::FINISHED;

    // if (!player->living->GetIsAttacking() && player->CanCast() && player->target && player->target->agent_id &&
    //     player->target->GetIsLivingType())
    //     AttackAgent(player->target);

    if (IsInVale(player))
    {
        const auto vale_rota = RoutineValeSpirits();
        if (vale_rota == RoutineState::FINISHED)
            return RoutineState::FINISHED;
    }

    const auto is_in_dhuum_room = IsInDhuumRoom(player);
    if (!is_in_dhuum_room)
        return RoutineState::FINISHED;

    auto dhuum_id = uint32_t{0};
    auto dhuum_hp = float{1.0F};
    const auto is_in_dhuum_fight = IsInDhuumFight(&dhuum_id, &dhuum_hp);

    if (was_in_dhuum_fight && !is_in_dhuum_fight)
        action_state = ActionState::INACTIVE;
    if (!is_in_dhuum_fight || !dhuum_id || dhuum_hp > 0.99F)
        return RoutineState::FINISHED;
    was_in_dhuum_fight = true;

    if (dhuum_hp < 0.25F)
        return RoutineState::FINISHED;

    // player->ChangeTarget(dhuum_id);
    // if (!player->living->GetIsAttacking() && player->target->agent_id)
    //     GW::CtoS::SendPacket(0x4, GAME_CMSG_ATTACK_AGENT);

    const auto pi_state = RoutinePI(dhuum_id);
    if (pi_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    const auto dhuum_rota = RoutineDhuum();
    if (dhuum_rota == RoutineState::FINISHED)
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

    if (IsUw())
    {
        const auto lt_id = GetTankId();
        if (lt_id)
            lt_agent = GW::Agents::GetAgentByID(lt_id);

        const auto emo_id = GetTankId();
        if (emo_id)
            emo_agent = GW::Agents::GetAgentByID(emo_id);
    }

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

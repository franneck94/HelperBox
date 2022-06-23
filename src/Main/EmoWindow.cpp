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

#include "EmoWindow.h"

namespace
{
static ActionState *emo_casting_action_state = nullptr;
static auto move_ongoing = false;

constexpr static auto DHUUM_JUDGEMENT_SKILL_ID = uint32_t{3085U};
constexpr static auto CANTHA_IDS = std::array<uint32_t, 4>{8990U, 8991U, 8992U, 8993U};
}; // namespace

EmoWindow::EmoWindow()
    : player({}), skillbar({}), fuse_pull(&player, &skillbar), pumping(&player, &skillbar, &bag_idx, &start_slot_idx),
      tank_bonding(&player, &skillbar), player_bonding(&player, &skillbar)
{
    if (skillbar.ValidateData())
        skillbar.Load();

    GW::StoC::RegisterPacketCallback<GW::Packet::StoC::MapLoaded>(
        &MapLoaded_Entry,
        [this](GW::HookStatus *status, GW::Packet::StoC::MapLoaded *packet) -> void {
            load_cb_triggered = ExplorableLoadCallback(status, packet);
        });
};

void EmoWindow::WarnDistanceLT() const
{
    static auto warned = false;
    constexpr auto warn_dist = GW::Constants::Range::Compass - 20.0F;

    const auto lt_id = GetTankId();
    const auto lt_agent = GW::Agents::GetAgentByID(lt_id);

    if (!lt_agent)
        return;

    const auto dist = GW::GetDistance(player.pos, lt_agent->pos);
    if (!warned && dist > warn_dist)
    {
        GW::Chat::WriteChat(GW::Chat::Channel::CHANNEL_GROUP, L"LT is leaving EMO's compass range!");
        warned = true;

        return;
    }

    if (warned && dist < warn_dist)
        warned = false;
}

void EmoWindow::Draw(IDirect3DDevice9 *pDevice)
{
    UNREFERENCED_PARAMETER(pDevice);

    if (!visible)
        return;

    if (!ActivationConditions())
        return;

    ImGui::SetNextWindowSize(ImVec2(110.0F, 330.0F), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("EmoWindow", nullptr, GetWinFlags()))
    {
        pumping.Draw();
        tank_bonding.Draw();
        player_bonding.Draw();
        fuse_pull.Draw();

        if (IsUw() || IsUwEntryOutpost())
        {
            DrawMovingButtons(moves, move_ongoing, move_idx);
        }
    }

    ImGui::End();

#ifdef _DEBUG
    if (IsUw() && show_debug_map)
        DrawMap(player, moves, move_idx, "DbMap");
#endif
}

void EmoWindow::UpdateUw()
{
    UpdateUwEntry();
    UpdateUwMoves();
    WarnDistanceLT();
}

void EmoWindow::UpdateUwEntry()
{
    static auto timer = clock();
    static auto triggered_tank_bonds_at_start = false;
    static auto triggered_move_start = false;

    if (load_cb_triggered)
    {
        tank_bonding.action_state = ActionState::ACTIVE;
        load_cb_triggered = false;
        triggered_tank_bonds_at_start = true;
        timer = clock();
        return;
    }

    if (triggered_tank_bonds_at_start && tank_bonding.action_state == ActionState::INACTIVE)
    {
        moves[0].Execute();
        triggered_tank_bonds_at_start = false;
        triggered_move_start = true;
        move_ongoing = true;
        return;
    }

    const auto timer_diff = TIMER_DIFF(timer);
    if (timer_diff < 500)
        return;

    if (triggered_move_start && move_idx == 1)
    {
        pumping.action_state = ActionState::ACTIVE;
        timer = 0;
    }
}

void EmoWindow::UpdateUwMoves()
{
    UpdatedUwMoves_Main(player, moves, move_idx, move_ongoing);
}

void EmoWindow::Update(float delta)
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

    emo_casting_action_state = &pumping.action_state;

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

    tank_bonding.Update();
    player_bonding.Update();
    fuse_pull.Update();
    pumping.Update();
}

bool EmoWindow::ActivationConditions() const
{
    if (!GW::Map::GetIsMapLoaded())
        return false;

    if (!GW::PartyMgr::GetIsPartyLoaded())
        return false;

    if (IsEmo(player))
        return true;

    return false;
}

Pumping::Pumping(Player *p, EmoSkillbar *s, uint32_t *_bag_idx, uint32_t *_start_slot_idx)
    : EmoActionABC(p, "Pumping", s), bag_idx(_bag_idx), start_slot_idx(_start_slot_idx)
{
    GW::StoC::RegisterPacketCallback<GW::Packet::StoC::AgentAdd>(
        &Summon_AgentAdd_Entry,
        [&](GW::HookStatus *, GW::Packet::StoC::AgentAdd *pak) -> void {
            if (pak->type != 1)
                return;

            const auto player_number = (pak->agent_type ^ 0x20000000);

            if (!IsUw() || player_number != 514) // Turtle id
                return;

            found_turtle = true;
            turtle_id = pak->agent_id;
        });
}

bool Pumping::RoutineSelfBonds() const
{
    const auto found_ether = player->HasEffect(GW::Constants::SkillID::Ether_Renewal);
    const auto found_sb = player->HasEffect(GW::Constants::SkillID::Spirit_Bond);
    const auto found_burning = player->HasEffect(GW::Constants::SkillID::Burning_Speed);

    if (player->CastEffectIfNotAvailable(skillbar->ether))
        return true;

    if (player->energy > 30U)
    {
        if (CastBondIfNotAvailable(skillbar->balth, player->id, player))
            return true;

        if (CastBondIfNotAvailable(skillbar->prot, player->id, player))
            return true;
    }

    if (found_ether && (player->hp_perc < 0.90F || !found_sb) && player->SpamEffect(skillbar->sb))
        return true;

    if (found_ether && (player->energy_perc < 0.90F || !found_burning) && player->SpamEffect(skillbar->burning))
        return true;

    return false;
}

bool Pumping::RoutineCanthaGuards() const
{
    static auto last_gdw_idx = 0;

    if (!player->CanCast())
        return false;

    const auto agents_array = GW::Agents::GetAgentArray();
    auto filtered_canthas = std::vector<GW::AgentLiving *>{};
    FilterAgents(*player,
                 agents_array,
                 filtered_canthas,
                 CANTHA_IDS,
                 GW::Constants::Allegiance::Npc_Minipet,
                 GW::Constants::Range::Spellcast);

    if (filtered_canthas.size() == 0)
        return false;

    for (const auto &cantha : filtered_canthas)
    {
        if (!cantha || cantha->GetIsDead() && cantha->hp == 0.0F)
            continue;

        if (cantha->hp < 0.90F && CastBondIfNotAvailable(skillbar->prot, cantha->agent_id, player))
            return true;

        if (cantha->hp < 0.70F && player->hp_perc > 0.5F)
            return (RoutineState::FINISHED == skillbar->fuse.Cast(player->energy, cantha->agent_id));

        if (!cantha->GetIsWeaponSpelled())
            return (RoutineState::FINISHED == skillbar->gdw.Cast(player->energy, cantha->agent_id));
    }

    return false;
}

bool Pumping::RoutineLT() const
{
    if (!lt_agent || !player->target || player->target->agent_id != lt_agent->agent_id)
        return false;

    const auto target_living = player->target->GetAsAgentLiving();
    if (!target_living || target_living->GetIsMoving() || player->living->GetIsMoving() ||
        target_living->primary != static_cast<uint8_t>(GW::Constants::Profession::Mesmer))
        return false;

    const auto dist = GW::GetDistance(player->pos, player->target->pos);

    if (dist < 1225.0F || dist > GW::Constants::Range::Spellcast)
        return false;

    if (target_living->hp < 0.80F && player->hp_perc > 0.5F)
        return (RoutineState::FINISHED == skillbar->fuse.Cast(player->energy, target_living->agent_id));

    if (target_living->hp < 0.99F)
        return (RoutineState::FINISHED == skillbar->sb.Cast(player->energy, target_living->agent_id));

    return false;
}

bool Pumping::RoutineDbAtDhuum() const
{
    if (!db_agent)
        return false;

    const auto dist = GW::GetDistance(player->pos, db_agent->pos);
    if (dist > GW::Constants::Range::Spellcast)
        return false;

    if (CastBondIfNotAvailable(skillbar->balth, db_agent->agent_id, player))
        return true;

    if (CastBondIfNotAvailable(skillbar->prot, db_agent->agent_id, player))
        return true;

    return false;
}

bool Pumping::RoutineTurtle() const
{
    if (!found_turtle || !turtle_id)
        return false;

    const auto turtle_agent = GW::Agents::GetAgentByID(turtle_id);
    if (!turtle_agent)
        return false;

    const auto turtle_living = turtle_agent->GetAsAgentLiving();
    if (!turtle_living)
        return false;

    const auto dist = GW::GetDistance(player->pos, turtle_agent->pos);
    if (dist > GW::Constants::Range::Spellcast)
        return false;

    if (CastBondIfNotAvailable(skillbar->prot, turtle_agent->agent_id, player))
        return true;

    if (CastBondIfNotAvailable(skillbar->life, turtle_agent->agent_id, player))
        return true;

    if (turtle_living->hp < 0.95F && player->hp_perc > 0.5F)
        return (RoutineState::FINISHED == skillbar->fuse.Cast(player->energy, turtle_agent->agent_id));

    return false;
}

bool Pumping::RoutinePI(const uint32_t dhuum_id) const
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

    if (dhuum_living->GetIsCasting() && dhuum_living->skill == DHUUM_JUDGEMENT_SKILL_ID)
        return (RoutineState::FINISHED == skillbar->pi.Cast(player->energy, dhuum_id));

    return false;
}

bool Pumping::RoutineWisdom() const
{
    return (RoutineState::FINISHED == skillbar->wisdom.Cast(player->energy));
}

bool Pumping::RoutineGDW() const
{
    static auto last_idx = uint32_t{0};

    if (last_idx >= GW::PartyMgr::GetPartySize())
        last_idx = 0;
    const auto id = party_members[last_idx].id;
    last_idx++;

    if (!player->CanCast() || !id || !party_data_valid || id == player->id)
        return false;

    const auto agent = GW::Agents::GetAgentByID(id);
    if (!agent)
        return false;

    const auto living = agent->GetAsAgentLiving();
    if (!living || living->GetIsMoving())
        return false;

    const auto dist = GW::GetDistance(player->pos, agent->pos);
    if (dist > GW::Constants::Range::Spellcast)
        return false;

    if (living->GetIsWeaponSpelled())
        return false;

    return (RoutineState::FINISHED == skillbar->gdw.Cast(player->energy, living->agent_id));
}

bool Pumping::RoutineTurtleGDW() const
{
    static auto last_idx = uint32_t{0};

    if (!player->CanCast())
        return false;

    const auto agent = GW::Agents::GetAgentByID(turtle_id);
    if (!agent)
        return false;

    const auto living = agent->GetAsAgentLiving();
    if (!living || living->GetIsMoving())
        return false;

    const auto dist = GW::GetDistance(player->pos, agent->pos);
    if (dist > GW::Constants::Range::Spellcast)
        return false;

    if (living->GetIsWeaponSpelled())
        return false;

    return (RoutineState::FINISHED == skillbar->gdw.Cast(player->energy, living->agent_id));
}

bool Pumping::RoutineDbBeforeDhuum() const
{
    if (!db_agent)
        return false;

    const auto living = db_agent->GetAsAgentLiving();
    if (!living)
        return false;

    const auto dist = GW::GetDistance(player->pos, db_agent->pos);
    if (dist > GW::Constants::Range::Spellcast)
        return false;

    if (living->hp > 0.75F)
        return false;

    if (CastBondIfNotAvailable(skillbar->prot, living->agent_id, player))
        return true;

    if (player->hp_perc > 0.5F)
        return (RoutineState::FINISHED == skillbar->fuse.Cast(player->energy, living->agent_id));

    return false;
}

bool Pumping::RoutineKeepPlayerAlive() const
{
    for (const auto &[id, _] : party_members)
    {
        if (!id || id == player->id)
            continue;

        const auto agent = GW::Agents::GetAgentByID(id);
        if (!agent)
            continue;

        const auto living = agent->GetAsAgentLiving();
        if (!living)
            continue;

        const auto dist = GW::GetDistance(player->pos, agent->pos);
        if (dist > GW::Constants::Range::Spellcast)
            continue;

        if (living->hp > 0.50F)
            continue;

        if (living->primary != static_cast<uint8_t>(GW::Constants::Profession::Ranger))
            if (CastBondIfNotAvailable(skillbar->prot, living->agent_id, player))
                return true;

        if (player->hp_perc > 0.5F)
            return (RoutineState::FINISHED == skillbar->fuse.Cast(player->energy, living->agent_id));
    }

    return false;
}

RoutineState Pumping::Routine()
{
    static auto timer_last_cast_ms = clock();
    static auto was_in_dhuum_fight = false;

    if (!player->CanCast())
        return RoutineState::FINISHED;

    const auto last_cast_diff_ms = TIMER_DIFF(timer_last_cast_ms);
    if (last_cast_diff_ms < 100)
        return RoutineState::FINISHED;
    timer_last_cast_ms = clock();

    if (RoutineSelfBonds())
        return RoutineState::FINISHED;

    if (!IsUw())
        return RoutineState::FINISHED;

    if (IsAtChamberSkele(player))
    {
        if (RoutineDbBeforeDhuum())
            return RoutineState::FINISHED;
    }

    if (IsAtFusePulls(player))
    {
        if (RoutineLT())
            return RoutineState::FINISHED;

        if (RoutineDbBeforeDhuum())
            return RoutineState::FINISHED;
    }

    if (IsInVale(player))
    {
        if (RoutineDbBeforeDhuum())
            return RoutineState::FINISHED;

        if (RoutineCanthaGuards())
            return RoutineState::FINISHED;
    }

    const auto is_in_dhuum_room = IsInDhuumRoom(player);
    if (!is_in_dhuum_room)
        return RoutineState::FINISHED;

    if (RoutineTurtle())
        return RoutineState::FINISHED;

    if (RoutineDbAtDhuum())
        return RoutineState::FINISHED;

    auto dhuum_id = uint32_t{0};
    auto dhuum_hp = float{1.0F};

    const auto is_in_dhuum_fight = IsInDhuumFight(&dhuum_id, &dhuum_hp);

    if (was_in_dhuum_fight && !is_in_dhuum_fight)
        *emo_casting_action_state = ActionState::INACTIVE;
    if (!is_in_dhuum_fight || !dhuum_id || dhuum_hp == 1.0F)
        return RoutineState::FINISHED;
    was_in_dhuum_fight = true;

    if (RoutineWisdom())
        return RoutineState::FINISHED;

    if (dhuum_hp < 0.20F)
        return RoutineState::FINISHED;

    if (RoutinePI(dhuum_id))
        return RoutineState::FINISHED;

    if (RoutineKeepPlayerAlive())
        return RoutineState::FINISHED;

    if (RoutineGDW())
        return RoutineState::FINISHED;

    if (RoutineTurtleGDW())
        return RoutineState::FINISHED;

    return RoutineState::FINISHED;
}

bool Pumping::PauseRoutine()
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

bool Pumping::ResumeRoutine()
{
    return !PauseRoutine();
}

void Pumping::Update()
{
    static auto paused = false;

    if (GW::PartyMgr::GetIsPartyDefeated())
        action_state = ActionState::INACTIVE;

    if (IsUw())
    {
        const auto lt_id = GetTankId();
        if (lt_id)
            lt_agent = GW::Agents::GetAgentByID(lt_id);

        const auto db_id = GetDhuumBitchId();
        if (db_id)
            db_agent = GW::Agents::GetAgentByID(db_id);
    }

    party_members.clear();
    party_data_valid = GetPartyMembers(party_members);

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

RoutineState TankBonding::Routine()
{
    static auto target_id = uint32_t{0};

    if (!player || !player->id || !player->CanCast())
    {
        target_id = 0;
        return RoutineState::ACTIVE;
    }

    if (interrupted)
    {
        target_id = 0;
        interrupted = false;
        return RoutineState::FINISHED;
    }

    // If no other player selected as target
    const auto no_target_or_self = (!player->target || player->target->agent_id == player->id);
    const auto target_not_self = (player->target && player->target->agent_id != player->id);

    // Get target at activation, after keep the id
    if (!target_id && no_target_or_self)
    {
        target_id = GetTankId();
        player->ChangeTarget(target_id);
    }
    else if (!target_id && target_not_self)
    {
        target_id = player->target->agent_id;
    }
    else if (!target_id)
    {
        target_id = 0;
        return RoutineState::FINISHED;
    }

    auto target = player->target;
    if (!target || target->agent_id == player->id)
    {
        player->ChangeTarget(target_id);
        target = player->target;
    }

    if (target->agent_id != target_id)
    {
        target_id = 0;
        return RoutineState::FINISHED;
    }

    const auto is_alive_ally = IsAliveAlly(target);
    if (!is_alive_ally)
    {
        target_id = 0;
        return RoutineState::FINISHED;
    }

    if (CastBondIfNotAvailable(skillbar->balth, target_id, player))
        return RoutineState::ACTIVE;

    if (CastBondIfNotAvailable(skillbar->prot, target_id, player))
        return RoutineState::ACTIVE;

    if (CastBondIfNotAvailable(skillbar->life, target_id, player))
        return RoutineState::ACTIVE;

    target_id = 0;
    return RoutineState::FINISHED;
}

void TankBonding::Update()
{
    if (GW::PartyMgr::GetIsPartyDefeated())
        action_state = ActionState::INACTIVE;

    if (action_state == ActionState::ACTIVE)
    {
        StateOnHold(*emo_casting_action_state);
        const auto routine_state = Routine();

        if (routine_state == RoutineState::FINISHED)
        {
            action_state = ActionState::INACTIVE;
            StateOnActive(*emo_casting_action_state);
        }
    }
}

RoutineState PlayerBonding::Routine()
{
    static auto target_id = uint32_t{0};

    if (interrupted)
    {
        target_id = 0;
        interrupted = false;
        return RoutineState::FINISHED;
    }

    if (!player->CanCast())
        return RoutineState::ACTIVE;

    if (!target_id && !player->target)
    {
        target_id = 0;
        return RoutineState::FINISHED;
    }

    if (!target_id)
        target_id = player->target->agent_id;

    auto target = player->target;
    if (!target || target->agent_id == player->id)
    {
        player->ChangeTarget(target_id);
        target = player->target;
    }

    if (target->agent_id != target_id)
    {
        target_id = 0;
        return RoutineState::FINISHED;
    }

    const auto is_alive_ally = IsAliveAlly(target);
    if (!is_alive_ally)
    {
        target_id = 0;
        return RoutineState::FINISHED;
    }

    if (CastBondIfNotAvailable(skillbar->balth, target_id, player))
        return RoutineState::ACTIVE;

    if (CastBondIfNotAvailable(skillbar->prot, target_id, player))
        return RoutineState::ACTIVE;

    target_id = 0;
    return RoutineState::FINISHED;
}

void PlayerBonding::Update()
{
    if (GW::PartyMgr::GetIsPartyDefeated())
        action_state = ActionState::INACTIVE;

    if (action_state == ActionState::ACTIVE)
    {
        StateOnHold(*emo_casting_action_state);
        const auto routine_state = Routine();

        if (routine_state == RoutineState::FINISHED)
        {
            action_state = ActionState::INACTIVE;
            StateOnActive(*emo_casting_action_state);
        }
    }
}

RoutineState FuseRange::Routine()
{
    ResetState(routine_state);

    if (!player->target)
    {
        ResetData();
        return RoutineState::FINISHED;
    }

    const auto is_alive_ally = IsAliveAlly(player->target);
    if (!is_alive_ally)
    {
        ResetData();
        return RoutineState::FINISHED;
    }

    const auto me_pos = player->pos;
    const auto target_pos = player->target->pos;

    if (routine_state == RoutineState::NONE)
    {
        const auto m_x = me_pos.x;
        const auto m_y = me_pos.y;
        const auto t_x = target_pos.x;
        const auto t_y = target_pos.y;

        const auto dist = GW::GetDistance(me_pos, target_pos);
        const auto d_t = FUSE_PULL_RANGE;
        const auto t = d_t / dist;

        const auto p_x = ((1.0F - t) * t_x + t * m_x);
        const auto p_y = ((1.0F - t) * t_y + t * m_y);

        requested_pos = GW::GamePos{p_x, p_y, 0};
        routine_state = SafeWalk(requested_pos, true);

        return RoutineState::ACTIVE;
    }
    else if (routine_state == RoutineState::ACTIVE && player->living->GetIsMoving())
    {
        return RoutineState::ACTIVE;
    }

    ResetData();
    return RoutineState::FINISHED;
}

void FuseRange::Update()
{
    if (GW::PartyMgr::GetIsPartyDefeated())
        action_state = ActionState::INACTIVE;

    if (action_state == ActionState::ACTIVE)
    {
        StateOnHold(*emo_casting_action_state);
        routine_state = Routine();

        if (routine_state == RoutineState::FINISHED)
        {
            action_state = ActionState::INACTIVE;
            StateOnActive(*emo_casting_action_state);
        }
    }
}

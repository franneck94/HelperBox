#include <cstdint>
#include <vector>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/Context/GameContext.h>
#include <GWCA/GameContainers/Array.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Party.h>
#include <GWCA/GameEntities/Player.h>
#include <GWCA/GameEntities/Skill.h>
#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/EffectMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Packets/Opcodes.h>

#include <Base/HelperBox.h>

#include <Actions.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <HelperAgents.h>
#include <HelperUw.h>
#include <HelperUwPos.h>
#include <Logger.h>
#include <MathUtils.h>
#include <PlayerData.h>
#include <SkillbarData.h>
#include <Timer.h>
#include <Types.h>

#include "EmoWindow.h"

namespace
{
static ActionState *emo_casting_action_state = nullptr;
static auto move_ongoing = false;

constexpr static auto DHUUM_JUDGEMENT_SKILL_ID = uint32_t{3085U};
constexpr static auto CANTHA_IDS =
    std::array<uint32_t, 4>{GW::Constants::ModelID::SummoningStone::ImperialBarrage,
                            GW::Constants::ModelID::SummoningStone::ImperialCripplingSlash,
                            GW::Constants::ModelID::SummoningStone::ImperialQuiveringBlade,
                            GW::Constants::ModelID::SummoningStone::ImperialTripleChop};
}; // namespace

EmoWindow::EmoWindow()
    : player_data({}), skillbar({}), pumping(&player_data, &skillbar, &bag_idx, &slot_idx, agents_data),
      tank_bonding(&player_data, &skillbar)
{
    if (skillbar.ValidateData())
        skillbar.Load();

    GW::StoC::RegisterPacketCallback<GW::Packet::StoC::MapLoaded>(
        &MapLoaded_Entry,
        [this](GW::HookStatus *status, GW::Packet::StoC::MapLoaded *packet) -> void {
            load_cb_triggered = ExplorableLoadCallback(status, packet);
        });
};

void EmoWindow::Draw(IDirect3DDevice9 *)
{
    if (!visible)
        return;

    if (!player_data.ValidateData(UwHelperActivationConditions))
        return;
    if (!IsEmo(player_data))
        return;

    ImGui::SetNextWindowSize(ImVec2(110.0F, 330.0F), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("EmoWindow", nullptr, GetWinFlags()))
    {
        pumping.Draw();

        if (IsUw() || IsUwEntryOutpost())
            DrawMovingButtons(moves, move_ongoing, move_idx);
    }

    ImGui::End();

#ifdef _DEBUG
    if (IsUw() && show_debug_map && agents_data)
        DrawMap(player_data.pos, agents_data->enemies, moves[move_idx]->pos, "DbMap");
#endif
}

void EmoWindow::UpdateUw()
{
    UpdateUwEntry();
    MoveABC::UpdatedUwMoves(player_data, agents_data, moves, move_idx, move_ongoing);
}

void EmoWindow::UpdateUwEntry()
{
    static auto timer = clock();
    static auto triggered_tank_bonds_at_start = false;
    static auto triggered_move_start = false;

    if (GW::PartyMgr::GetPartySize() >= 7)
        load_cb_triggered = false;

    if (load_cb_triggered)
    {
        move_idx = 0;
        tank_bonding.action_state = ActionState::ACTIVE;
        load_cb_triggered = false;
        triggered_tank_bonds_at_start = true;
        timer = clock();
        return;
    }

    if (triggered_tank_bonds_at_start && tank_bonding.action_state == ActionState::INACTIVE)
    {
        moves[0]->Execute();
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

void EmoWindow::Update(float, const AgentLivingData &_agents_data)
{
    if (!player_data.ValidateData(UwHelperActivationConditions))
        return;
    player_data.Update();
    agents_data = &_agents_data;

    if (!IsEmo(player_data))
        return;

    if (IsUw() && first_frame)
    {
        UpdateUwInfo(player_data, moves, move_idx, true);
        first_frame = false;
    }

    if (!skillbar.ValidateData())
        return;
    skillbar.Update();

    emo_casting_action_state = &pumping.action_state;

    if (IsUw())
    {
        UpdateUwInfo(player_data, moves, move_idx, false);
        UpdateUw();
    }

    tank_bonding.Update();
    pumping.Update();
}

Pumping::Pumping(PlayerData *p, EmoSkillbarData *s, uint32_t *_bag_idx, uint32_t *_slot_idx, const AgentLivingData *a)
    : EmoActionABC(p, "Pumping", s), bag_idx(_bag_idx), slot_idx(_slot_idx), agents_data(a)
{
    GW::StoC::RegisterPacketCallback<GW::Packet::StoC::AgentAdd>(
        &Summon_AgentAdd_Entry,
        [&](GW::HookStatus *, GW::Packet::StoC::AgentAdd *pak) -> void {
            if (pak->type != 1)
                return;

            const auto player_number = (pak->agent_type ^ 0x20000000);

            if (!IsUw() || player_number != GW::Constants::ModelID::SummoningStone::JadeiteSiegeTurtle)
                return;

            found_turtle = true;
            turtle_id = pak->agent_id;
        });
}

bool Pumping::RoutineWhenInRangeBondLT() const
{
    if (!lt_agent)
        return false;

    const auto dist = GW::GetDistance(player_data->pos, lt_agent->pos);
    if (dist > GW::Constants::Range::Spellcast)
        return false;

    const auto lt_living = lt_agent->GetAsAgentLiving();
    if (!lt_living)
        return false;

    if (lt_living->GetIsMoving() || player_data->living->GetIsMoving())
        return false;

    if (CastBondIfNotAvailable(skillbar->balth, lt_agent->agent_id, player_data))
        return true;

    if (CastBondIfNotAvailable(skillbar->prot, lt_agent->agent_id, player_data))
        return true;

    if (CastBondIfNotAvailable(skillbar->life, lt_agent->agent_id, player_data))
        return true;

    return false;
}

bool Pumping::RoutineSelfBonds() const
{
    const auto found_ether = player_data->HasEffect(GW::Constants::SkillID::Ether_Renewal);
    const auto found_sb = player_data->HasEffect(GW::Constants::SkillID::Spirit_Bond);
    const auto found_burning = player_data->HasEffect(GW::Constants::SkillID::Burning_Speed);

    if (player_data->CastEffectIfNotAvailable(skillbar->ether))
        return true;

    if (player_data->energy > 30U)
    {
        if (CastBondIfNotAvailable(skillbar->balth, player_data->id, player_data))
            return true;

        if (CastBondIfNotAvailable(skillbar->prot, player_data->id, player_data))
            return true;
    }

    if (found_ether && (player_data->hp_perc < 0.85F || !found_sb) && player_data->SpamEffect(skillbar->sb))
        return true;

    if (found_ether && (player_data->energy_perc < 0.85F || !found_burning) &&
        player_data->SpamEffect(skillbar->burning))
        return true;

    return false;
}

bool Pumping::RoutineCanthaGuards() const
{
    static auto last_gdw_idx = 0;

    if (!player_data->CanCast())
        return false;

    if (!agents_data)
        return true;

    const auto filtered_enemies =
        FilterAgentsByRange(agents_data->enemies, *player_data, GW::Constants::Range::Earshot);

    auto filtered_canthas = std::vector<GW::AgentLiving *>{};
    FilterAgents(*player_data,
                 filtered_canthas,
                 CANTHA_IDS,
                 GW::Constants::Allegiance::Npc_Minipet,
                 GW::Constants::Range::Spellcast);

    if (filtered_canthas.size() == 0)
        return false;

    if (filtered_enemies.size() > 0)
    {
        for (const auto cantha : filtered_canthas)
        {
            if (!cantha || cantha->GetIsDead() && cantha->hp == 0.00F)
                continue;

            if (cantha->hp < 0.90F && CastBondIfNotAvailable(skillbar->prot, cantha->agent_id, player_data))
                return true;

            if (cantha->hp < 0.70F && player_data->hp_perc > 0.05F)
                return (RoutineState::FINISHED == skillbar->fuse.Cast(player_data->energy, cantha->agent_id));

            if (!cantha->GetIsWeaponSpelled())
                return (RoutineState::FINISHED == skillbar->gdw.Cast(player_data->energy, cantha->agent_id));
        }

        if (db_agent && db_agent->agent_id)
        {
            const auto dist = GW::GetDistance(db_agent->pos, player_data->pos);
            if (dist < GW::Constants::Range::Spellcast)
                return (RoutineState::FINISHED == skillbar->gdw.Cast(player_data->energy, db_agent->agent_id));
        }
    }
    else // Done at vale, drop buffs
    {
        auto buffs = GW::Effects::GetPlayerBuffs();
        if (!buffs || !buffs->valid() || buffs->size() == 0)
            return false;

        for (const auto &buff : *buffs)
        {
            const auto agent_id = buff.target_agent_id;
            const auto skill = static_cast<GW::Constants::SkillID>(buff.skill_id);
            const auto is_prot_bond = skill == GW::Constants::SkillID::Protective_Bond;
            const auto cantha_it = std::find_if(filtered_canthas.begin(),
                                                filtered_canthas.end(),
                                                [=](const auto living) { return living->agent_id == agent_id; });
            if (is_prot_bond && cantha_it != filtered_canthas.end())
                GW::Effects::DropBuff(buff.buff_id);
        }

        return true;
    }

    return false;
}

bool Pumping::RoutineLT() const
{
    static auto last_time_sb_ms = clock();

    if (!lt_agent || !player_data->target || player_data->target->agent_id != lt_agent->agent_id)
        return false;

    const auto target_living = player_data->target->GetAsAgentLiving();
    if (!target_living || target_living->GetIsMoving() || player_data->living->GetIsMoving() ||
        target_living->primary != static_cast<uint8_t>(GW::Constants::Profession::Mesmer))
        return false;

    const auto dist = GW::GetDistance(player_data->pos, player_data->target->pos);

    if (dist < 1225.0F || dist > GW::Constants::Range::Spellcast)
        return false;

    if (target_living->hp < 0.80F && player_data->hp_perc > 0.5F)
        return (RoutineState::FINISHED == skillbar->fuse.Cast(player_data->energy, target_living->agent_id));

    if (TIMER_DIFF(last_time_sb_ms) > 4'000L &&
        RoutineState::FINISHED == skillbar->sb.Cast(player_data->energy, target_living->agent_id))
    {
        last_time_sb_ms = clock();
        return true;
    }

    return false;
}

bool Pumping::RoutineDbAtDhuum() const
{
    if (!db_agent)
        return false;

    const auto dist = GW::GetDistance(player_data->pos, db_agent->pos);
    if (dist > GW::Constants::Range::Spellcast)
        return false;

    if (CastBondIfNotAvailable(skillbar->balth, db_agent->agent_id, player_data))
        return true;

    if (CastBondIfNotAvailable(skillbar->prot, db_agent->agent_id, player_data))
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

    const auto dist = GW::GetDistance(player_data->pos, turtle_agent->pos);
    if (dist > GW::Constants::Range::Spellcast)
        return false;

    if (CastBondIfNotAvailable(skillbar->prot, turtle_agent->agent_id, player_data))
        return true;

    if (CastBondIfNotAvailable(skillbar->life, turtle_agent->agent_id, player_data))
        return true;

    if (turtle_living->hp < 0.80F && player_data->hp_perc > 0.25F)
        return (RoutineState::FINISHED == skillbar->fuse.Cast(player_data->energy, turtle_agent->agent_id));
    else if (turtle_living->hp < 0.95F && player_data->hp_perc > 0.5F)
        return (RoutineState::FINISHED == skillbar->fuse.Cast(player_data->energy, turtle_agent->agent_id));
    else if (turtle_living->hp < 0.99F && player_data->hp_perc > 0.75F)
        return (RoutineState::FINISHED == skillbar->fuse.Cast(player_data->energy, turtle_agent->agent_id));
    else if (turtle_living->hp < 0.95F)
        return (RoutineState::FINISHED == skillbar->sb.Cast(player_data->energy, turtle_agent->agent_id));

    return false;
}

bool Pumping::RoutineWisdom() const
{
    return (RoutineState::FINISHED == skillbar->wisdom.Cast(player_data->energy));
}

bool Pumping::RoutineGDW() const
{
    static auto last_idx = uint32_t{0};

    if (last_idx >= GW::PartyMgr::GetPartySize())
        last_idx = 0;
    const auto id = party_members[last_idx].id;
    last_idx++;

    if (!player_data->CanCast() || !id || !party_data_valid || id == player_data->id)
        return false;

    const auto agent = GW::Agents::GetAgentByID(id);
    if (!agent)
        return false;

    const auto living = agent->GetAsAgentLiving();
    if (!living || living->GetIsMoving())
        return false;

    const auto dist = GW::GetDistance(player_data->pos, agent->pos);
    if (dist > 450.0F)
        return false;

    if (living->GetIsWeaponSpelled())
        return false;

    return (RoutineState::FINISHED == skillbar->gdw.Cast(player_data->energy, living->agent_id));
}

bool Pumping::RoutineTurtleGDW() const
{
    static auto last_idx = uint32_t{0};

    if (!player_data->CanCast())
        return false;

    const auto agent = GW::Agents::GetAgentByID(turtle_id);
    if (!agent)
        return false;

    const auto living = agent->GetAsAgentLiving();
    if (!living || living->GetIsMoving())
        return false;

    const auto dist = GW::GetDistance(player_data->pos, agent->pos);
    if (dist > GW::Constants::Range::Spellcast)
        return false;

    if (living->GetIsWeaponSpelled())
        return false;

    return (RoutineState::FINISHED == skillbar->gdw.Cast(player_data->energy, living->agent_id));
}

bool Pumping::RoutineDbBeforeDhuum() const
{
    if (!db_agent)
        return false;

    const auto living = db_agent->GetAsAgentLiving();
    if (!living)
        return false;

    const auto dist = GW::GetDistance(player_data->pos, db_agent->pos);
    if (dist > 2100.0F)
        return false;

    if (living->hp > 0.75F)
        return false;

    if (CastBondIfNotAvailable(skillbar->prot, living->agent_id, player_data))
        return true;

    if (living->hp < 0.50F && player_data->hp_perc > 0.50F)
        return (RoutineState::FINISHED == skillbar->fuse.Cast(player_data->energy, living->agent_id));

    return false;
}

bool Pumping::RoutineKeepPlayerAlive() const
{
    if (player_data->living->GetIsMoving())
        return false;

    if (player_data->energy < 50U)
        return false;

    for (const auto &[id, _] : party_members)
    {
        if (!id || id == player_data->id)
            continue;

        const auto agent = GW::Agents::GetAgentByID(id);
        if (!agent)
            continue;

        const auto living = agent->GetAsAgentLiving();
        if (!living)
            continue;

        const auto dist = GW::GetDistance(player_data->pos, agent->pos);
        if (dist > 450.0F)
            continue;

        if (living->hp > 0.50F)
            continue;

        if (living->primary != static_cast<uint8_t>(GW::Constants::Profession::Ranger))
            if (CastBondIfNotAvailable(skillbar->prot, living->agent_id, player_data))
                return true;

        if (player_data->hp_perc > 0.50F)
            return (RoutineState::FINISHED == skillbar->fuse.Cast(player_data->energy, living->agent_id));
    }

    return false;
}

bool Pumping::DropBondsLT() const
{
    if (!lt_agent || !lt_agent->agent_id)
        return false;

    auto buffs = GW::Effects::GetPlayerBuffs();
    if (!buffs || !buffs->valid())
        return false;

    for (const auto &buff : *buffs)
    {
        const auto agent_id = buff.target_agent_id;
        const auto skill = static_cast<GW::Constants::SkillID>(buff.skill_id);
        const auto is_prot_bond = skill == GW::Constants::SkillID::Protective_Bond;
        const auto is_life_bond = skill == GW::Constants::SkillID::Life_Bond;
        const auto is_balth_bond = skill == GW::Constants::SkillID::Balthazars_Spirit;

        if ((is_prot_bond || is_life_bond || is_balth_bond) && agent_id == lt_agent->agent_id)
        {
            GW::Effects::DropBuff(buff.buff_id);
            return true;
        }
    }

    return false;
}

RoutineState Pumping::Routine()
{
    const auto is_in_dhuum_room = IsInDhuumRoom(player_data->pos);

    if (!player_data->CanCast())
        return RoutineState::ACTIVE;

    if (!ActionABC::HasWaitedLongEnough())
        return RoutineState::ACTIVE;

    if (!is_in_dhuum_room && RoutineWhenInRangeBondLT())
        return RoutineState::FINISHED;

    if (RoutineSelfBonds())
        return RoutineState::FINISHED;

    if (!IsUw())
        return RoutineState::FINISHED;

    if (IsAtSpawn(player_data->pos) && RoutineKeepPlayerAlive())
        return RoutineState::FINISHED;

    if (!is_in_dhuum_room && RoutineDbBeforeDhuum())
        return RoutineState::FINISHED;

    if (IsAtFusePulls(player_data->pos) && RoutineLT())
        return RoutineState::FINISHED;

    if (IsAtValeSpirits(player_data->pos) && RoutineCanthaGuards())
        return RoutineState::FINISHED;

    if (IsGoingToDhuum(player_data->pos) && DropBondsLT())
        return RoutineState::FINISHED;

    if (!is_in_dhuum_room)
        return RoutineState::FINISHED;

    if (RoutineTurtle())
        return RoutineState::FINISHED;

    if (RoutineDbAtDhuum())
        return RoutineState::FINISHED;

    auto dhuum_id = uint32_t{0};
    auto dhuum_hp = float{1.0F};

    const auto is_in_dhuum_fight = IsInDhuumFight(&dhuum_id, &dhuum_hp);

    if (!is_in_dhuum_fight || !dhuum_id)
        return RoutineState::FINISHED;

    if (RoutineWisdom())
        return RoutineState::FINISHED;

    if (dhuum_hp < 0.20F)
        return RoutineState::FINISHED;

    if (DhuumIsCastingJudgement(dhuum_id) &&
        (RoutineState::FINISHED == skillbar->pi.Cast(player_data->energy, dhuum_id)))
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
    if (player_data->living->GetIsMoving())
        return true;

    if (player_data->target)
    {
        if (TargetIsReaper(*player_data) && (GW::GetDistance(player_data->pos, player_data->target->pos) < 200.0F) &&
            player_data->energy_perc > 0.30F)
            return true;
    }

    return false;
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

    if (action_state == ActionState::INACTIVE && player_data->living && player_data->living->GetIsIdle() &&
        (player_data->energy_perc < 0.30F || player_data->hp_perc < 0.30F))
        action_state = ActionState::ACTIVE;

    if (action_state == ActionState::ACTIVE)
        (void)Routine();
}

RoutineState TankBonding::Routine()
{
    static auto target_id = uint32_t{0};

    if (!player_data || !player_data->id || !player_data->CanCast())
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

    // If no other player_data selected as target
    const auto no_target_or_self = (!player_data->target || player_data->target->agent_id == player_data->id);
    const auto target_not_self = (player_data->target && player_data->target->agent_id != player_data->id);

    // Get target at activation, after keep the id
    if (!target_id && no_target_or_self)
    {
        target_id = GetTankId();
        player_data->ChangeTarget(target_id);
    }
    else if (!target_id && target_not_self)
    {
        target_id = player_data->target->agent_id;
    }
    else if (!target_id)
    {
        target_id = 0;
        return RoutineState::FINISHED;
    }

    auto target = player_data->target;
    if (!target || target->agent_id == player_data->id)
    {
        player_data->ChangeTarget(target_id);
        target = player_data->target;
    }
    if (!player_data->target)
        return RoutineState::FINISHED;

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

    if (CastBondIfNotAvailable(skillbar->balth, target_id, player_data))
        return RoutineState::ACTIVE;

    if (CastBondIfNotAvailable(skillbar->prot, target_id, player_data))
        return RoutineState::ACTIVE;

    if (CastBondIfNotAvailable(skillbar->life, target_id, player_data))
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

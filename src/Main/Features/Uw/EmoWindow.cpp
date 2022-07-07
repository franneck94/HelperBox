#include <cstdint>
#include <vector>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/Context/GameContext.h>
#include <GWCA/Context/WorldContext.h>
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

#include <ActionsUw.h>
#include <Base/HelperBox.h>
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
static auto lt_is_ready = false;

constexpr static auto DHUUM_JUDGEMENT_SKILL_ID = uint32_t{3085U};
constexpr static auto CANTHA_IDS =
    std::array<uint32_t, 4>{GW::Constants::ModelID::SummoningStone::ImperialBarrage,
                            GW::Constants::ModelID::SummoningStone::ImperialCripplingSlash,
                            GW::Constants::ModelID::SummoningStone::ImperialQuiveringBlade,
                            GW::Constants::ModelID::SummoningStone::ImperialTripleChop};
constexpr static auto ESCORT_IDS = std::array<uint32_t, 6>{GW::Constants::ModelID::UW::Escort1,
                                                           GW::Constants::ModelID::UW::Escort2,
                                                           GW::Constants::ModelID::UW::Escort3,
                                                           GW::Constants::ModelID::UW::Escort4,
                                                           GW::Constants::ModelID::UW::Escort5,
                                                           GW::Constants::ModelID::UW::Escort6};
constexpr static auto CANTHA_STONE_ID = uint32_t{30210};
constexpr static auto COOKIE_ID = uint32_t{28433};
}; // namespace

EmoWindow::EmoWindow()
    : player_data({}), skillbar({}), pumping(&player_data, &skillbar, &bag_idx, &slot_idx, agents_data),
      tank_bonding(&player_data, &skillbar)
{
    if (skillbar.ValidateData())
        skillbar.Load();

    GW::StoC::RegisterPacketCallback(&SendChat_Entry,
                                     GAME_SMSG_CHAT_MESSAGE_LOCAL,
                                     [this](GW::HookStatus *status, GW::Packet::StoC::PacketBase *packet) -> void {
                                         lt_is_ready = OnChatMessageLtIsReady(status, packet, TriggerRole::LT);
                                     });

    GW::StoC::RegisterPacketCallback<GW::Packet::StoC::MapLoaded>(
        &MapLoaded_Entry,
        [this](GW::HookStatus *status, GW::Packet::StoC::MapLoaded *packet) -> void {
            load_cb_triggered = ExplorableLoadCallback(status, packet);
            num_finished_objectives = 0U;
        });

    GW::StoC::RegisterPacketCallback<GW::Packet::StoC::ObjectiveDone>(
        &ObjectiveDone_Entry,
        [this](GW::HookStatus *, GW::Packet::StoC::ObjectiveDone *packet) {
            ++num_finished_objectives;
            Log::Info("Finished Objective : %u, Num objectives: %u", packet->objective_id, num_finished_objectives);
        });
};

void EmoWindow::Draw(IDirect3DDevice9 *)
{
    if (!visible || !player_data.ValidateData(UwHelperActivationConditions) || !IsEmo(player_data))
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
        DrawMap(player_data.pos, agents_data->enemies, moves[move_idx]->pos, "EmoMap");
#endif
}

void EmoWindow::UpdateUw()
{
    UpdateUwEntry();
    MoveABC::UpdatedUwMoves(player_data, agents_data, moves, move_idx, move_ongoing);

    if (num_finished_objectives == 10U && !move_ongoing && moves[move_idx]->name == "Go To Dhuum 1")
    {
        moves[move_idx]->Execute();
        move_ongoing = true;
    }

    if (lt_is_ready && !move_ongoing &&
        (moves[move_idx]->name == "Talk Lab Reaper" || moves[move_idx]->name == "Go Wastes 1" ||
         moves[move_idx]->name == "Go To Dhuum 1"))
    {
        moves[move_idx]->Execute();
        move_ongoing = true;
        lt_is_ready = false;
    }
    if (move_ongoing)
        lt_is_ready = false;
}

void EmoWindow::UpdateUwEntry()
{
    static auto timer = clock();
    static auto triggered_tank_bonds_at_start = false;
    static auto triggered_move_start = false;

    if (TankIsFullteamLT())
        load_cb_triggered = false;

    if (load_cb_triggered)
    {
        move_idx = 0;
        num_finished_objectives = 0U;
        move_ongoing = false;
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

    if (triggered_move_start && move_idx == 1)
    {
        pumping.action_state = ActionState::ACTIVE;
        timer = 0;
    }
}

void EmoWindow::Update(float, const AgentLivingData &_agents_data)
{
    if (!player_data.ValidateData(UwHelperActivationConditions))
    {
        move_idx = 0;
        move_ongoing = false;
        tank_bonding.action_state = ActionState::INACTIVE;
        return;
    }
    player_data.Update();
    agents_data = &_agents_data;
    pumping.agents_data = agents_data;

    if (!IsEmo(player_data))
        return;

    if (IsUw() && first_frame)
    {
        UpdateUwInfo(player_data, moves, move_idx, true, move_ongoing);
        first_frame = false;
    }

    if (!skillbar.ValidateData())
        return;
    skillbar.Update();

    emo_casting_action_state = &pumping.action_state;

    if (IsUw())
    {
        UpdateUwInfo(player_data, moves, move_idx, false, move_ongoing);
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

    if (player_data->energy < 40U)
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

bool Pumping::RoutineEscortSpirits() const
{
    if (!agents_data || agents_data->npcs.size() == 0)
        return false;

    auto spirits_livings = std::vector<GW::AgentLiving *>{};
    FilterByIdsAndDistances(player_data->pos, agents_data->npcs, spirits_livings, ESCORT_IDS);

    if (spirits_livings.size() == 0)
        return false;

    for (const auto spirit : spirits_livings)
    {
        if (!spirit)
            continue;

        if (spirit->hp > 0.80F)
            continue;

        const auto dist = GW::GetDistance(player_data->pos, spirit->pos);
        const auto is_far_away = dist > 3000.0F;

        if (spirit->hp < 0.60F || is_far_away)
            return (RoutineState::FINISHED == skillbar->fuse.Cast(player_data->energy, spirit->agent_id));
        else if (player_data->hp_perc > 0.50F)
            return (RoutineState::FINISHED == skillbar->fuse.Cast(player_data->energy, spirit->agent_id));
    }

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
    FilterByIdsAndDistances(player_data->pos,
                            agents_data->npcs,
                            filtered_canthas,
                            CANTHA_IDS,
                            GW::Constants::Range::Spellcast);

    if (filtered_canthas.size() == 0)
        return false;

    if (filtered_enemies.size() > 0)
    {
        for (const auto cantha : filtered_canthas)
        {
            if (!cantha || cantha->GetIsDead())
                continue;

            if (cantha->hp < 0.90F && CastBondIfNotAvailable(skillbar->prot, cantha->agent_id, player_data))
                return true;

            if (cantha->hp < 0.70F && player_data->hp_perc > 0.50F)
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

        auto droppped_smth = false;
        for (auto living : filtered_canthas)
        {
            if (DropBondsOnLiving(living))
                droppped_smth = true;
        }

        if (droppped_smth)
            return true;
    }

    return false;
}

bool Pumping::RoutineLT() const
{
    static auto last_time_sb_ms = clock();
    static auto casted_fuse_once = false;

    if (!lt_agent || !player_data->target || player_data->target->agent_id != lt_agent->agent_id)
    {
        casted_fuse_once = false;
        return false;
    }

    const auto target_living = player_data->target->GetAsAgentLiving();
    if (!target_living || target_living->GetIsMoving() || player_data->living->GetIsMoving() ||
        target_living->primary != static_cast<uint8_t>(GW::Constants::Profession::Mesmer))
    {
        casted_fuse_once = false;
        return false;
    }

    const auto dist = GW::GetDistance(player_data->pos, player_data->target->pos);

    const auto min_range_fuse = 1220.0F;
    if (dist < min_range_fuse || dist > GW::Constants::Range::Spellcast)
    {
        casted_fuse_once = false;
        return false;
    }

    if (!casted_fuse_once || (target_living->hp < 0.90F && player_data->hp_perc > 0.50F))
    {
        if (RoutineState::FINISHED == skillbar->fuse.Cast(player_data->energy, target_living->agent_id))
        {
            casted_fuse_once = true;
            return true;
        }
    }

    const auto sb_recast_threshold_ms = 6'000L;
    if (TIMER_DIFF(last_time_sb_ms) > sb_recast_threshold_ms &&
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

    const auto dist = GW::GetDistance(GW::GamePos{-16410.75F, 17294.47F, 0}, agent->pos);
    if (dist > GW::Constants::Range::Area)
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

    const auto dist = GW::GetDistance(GW::GamePos{-16105.50F, 17284.84F, 0}, agent->pos);
    if (dist > GW::Constants::Range::Spellcast)
        return false;

    if (living->GetIsWeaponSpelled())
        return false;

    return (RoutineState::FINISHED == skillbar->gdw.Cast(player_data->energy, living->agent_id));
}

bool Pumping::RoutineDbBeforeDhuum() const
{
    static auto last_time_sb_ms = clock();

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

    const auto sb_recast_threshold_ms = 6'000L;
    if (TIMER_DIFF(last_time_sb_ms) > sb_recast_threshold_ms && living->hp < 0.50F &&
        RoutineState::FINISHED == skillbar->sb.Cast(player_data->energy, living->agent_id))
    {
        last_time_sb_ms = clock();
        return true;
    }

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
    if (!buffs || !buffs->valid() || buffs->size() == 0)
        return false;

    const auto lt_living = lt_agent->GetAsAgentLiving();
    if (!lt_living)
        return false;

    if (DropBondsOnLiving(lt_living))
        return true;

    return false;
}

RoutineState Pumping::Routine()
{
    static bool used_canthas = false;
    const auto is_in_dhuum_room = IsInDhuumRoom(player_data->pos);

    if (!player_data->CanCast())
        return RoutineState::ACTIVE;

    if (!ActionABC::HasWaitedLongEnough())
        return RoutineState::ACTIVE;

    if (RoutineSelfBonds())
        return RoutineState::FINISHED;

    if (!is_in_dhuum_room && RoutineWhenInRangeBondLT())
        return RoutineState::FINISHED;

    if (!IsUw())
    {
        used_canthas = false;
        return RoutineState::FINISHED;
    }

    if (IsAtSpawn(player_data->pos) && RoutineKeepPlayerAlive())
        return RoutineState::FINISHED;

    if (!is_in_dhuum_room && RoutineDbBeforeDhuum())
        return RoutineState::FINISHED;

    if (TankIsFullteamLT() && !is_in_dhuum_room)
        return RoutineState::FINISHED;

    if ((IsInBasement(player_data->pos) || IsInVale(player_data->pos)) && RoutineEscortSpirits())
        return RoutineState::FINISHED;

    if (IsAtFusePulls(player_data->pos) && RoutineLT())
        return RoutineState::FINISHED;

    if (IsAtValeSpirits(player_data->pos) && GW::PartyMgr::GetPartySize() < 6 && !used_canthas &&
        UseInventoryItem(CANTHA_STONE_ID, 1, 5))
    {
        used_canthas = true;
        return RoutineState::FINISHED;
    }

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

    if (!is_in_dhuum_fight && (DhuumFightDone(agents_data->npcs) || DhuumFightDone(agents_data->neutrals)))
    {
        action_state = ActionState::INACTIVE;
        move_ongoing = false;
        used_canthas = false;
    }

    if (!is_in_dhuum_fight || !dhuum_id)
        return RoutineState::FINISHED;

    const auto current_morale = GW::GameContext::instance()->world->morale;
    if (current_morale <= 90 && UseInventoryItem(COOKIE_ID, 1, 5))
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

    if (player_data->target && TargetIsReaper(*player_data))
    {
        const auto dist_reaper = GW::GetDistance(player_data->pos, player_data->target->pos);
        if (dist_reaper < GW::Constants::Range::Nearby && player_data->energy > 30U)
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
    if (!player_data || !player_data->id || !player_data->CanCast())
        return RoutineState::ACTIVE;

    auto target_id = GetTankId();
    if (!target_id)
        return RoutineState::FINISHED;
    player_data->ChangeTarget(target_id);

    auto target = player_data->target;
    if (!target)
        return RoutineState::FINISHED;

    const auto is_alive_ally = IsAliveAlly(target);
    if (!is_alive_ally)
        return RoutineState::FINISHED;

    if (CastBondIfNotAvailable(skillbar->balth, target_id, player_data))
        return RoutineState::ACTIVE;

    if (CastBondIfNotAvailable(skillbar->prot, target_id, player_data))
        return RoutineState::ACTIVE;

    if (CastBondIfNotAvailable(skillbar->life, target_id, player_data))
        return RoutineState::ACTIVE;

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
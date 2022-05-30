#include "stdafx.h"

#include <array>
#include <cstdint>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/Context/GameContext.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Party.h>
#include <GWCA/GameEntities/Player.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/EffectMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Managers/PlayerMgr.h>
#include <GWCA/Managers/SkillbarMgr.h>
#include <GWCA/Packets/Opcodes.h>

#include <fmt/format.h>

#include <HelperBox.h>
#include <Logger.h>
#include <Timer.h>

#include <Actions.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <Player.h>
#include <Skillbars.h>
#include <Types.h>
#include <Utils.h>

#include "EmoWindow.h"

namespace
{
static constexpr auto MIN_CYCLE_TIME_MS = uint32_t{100};

static ActionState *emo_casting_action_state = nullptr;
static bool send_move = false;
static bool reset_template = false;
}; // namespace

void Move::Execute()
{
    if (!CanMove())
        return;
    GW::Agent *me = GW::Agents::GetPlayer();
    double dist = GW::GetDistance(me->pos, GW::Vec2f(x, y));
    if (range != 0 && dist > range)
        return;
    GW::Agents::Move(x, y);
    Log::Info("Moving to (%.0f, %.0f)", x, y);

    send_move = true;
}

void ActionABC::Draw(const ImVec2 button_size)
{
    if (!IsExplorable())
        action_state = ActionState::INACTIVE;

    const auto color = COLOR_MAPPING[static_cast<uint32_t>(action_state)];
    DrawButton(action_state, color, text, button_size);
}

EmoWindow::EmoWindow()
    : player({}), skillbar({}), fuse_pull(&player, &skillbar), pumping(&player, &skillbar),
      tank_bonding(&player, &skillbar), player_bonding(&player, &skillbar), moves({})
{
    if (skillbar.ValidateData())
        skillbar.Load();

    GW::StoC::RegisterPacketCallback<GW::Packet::StoC::MapLoaded>(
        &MapLoaded_Entry,
        [this](GW::HookStatus *status, GW::Packet::StoC::MapLoaded *packet) -> void {
            UNREFERENCED_PARAMETER(status);
            UNREFERENCED_PARAMETER(packet);
            switch (GW::Map::GetInstanceType())
            {
            case GW::Constants::InstanceType::Explorable:
                reset_template = true;
                break;
            case GW::Constants::InstanceType::Outpost:
            case GW::Constants::InstanceType::Loading:
            default:
                reset_template = false;
                break;
            }
        });

    moves.push_back(Move{1248.0F, 6965.509766F, 5000.0F, "Spawn"});
    moves.push_back(Move{-583.28F, 9275.68F, 5000.0F, "Lab Stairs1"});
    moves.push_back(Move{-2730.79F, 10159.21F, 5000.0F, "Lab Stairs2"});
    moves.push_back(Move{-5683.589844F, 12662.990234F, 5000.0F, "Lab Reaper"});
    moves.push_back(Move{-6459.410156F, 9943.219727F, 5000.0F, "Fuse Pull 1"});
    moves.push_back(Move{-6241.24F, 7945.73F, 5000.0F, "Basement"});
    moves.push_back(Move{-8763.36F, 5551.18F, 5000.0F, "Basement Stairs"});
    moves.push_back(Move{-7980.55F, 4308.90F, 5000.0F, "Fuse Pull 2"});
    moves.push_back(Move{-8764.08F, 2156.60F, 5000.0F, "Vale Entry"});
    moves.push_back(Move{-12264.129883F, 1821.180054F, 5000.0F, "Vale House"});
    moves.push_back(Move{-13872.34F, 2332.34F, 5000.0F, "Spirits 1"});
    moves.push_back(Move{-13760.19F, 358.15F, 5000.0F, "Spirits 2"});
    moves.push_back(Move{-12145.44F, 1101.74F, 5000.0F, "Spirits 3"});
    moves.push_back(Move{-8764.08F, 2156.60F, 5000.0F, "Vale Entry"});
    moves.push_back(Move{-7980.55F, 4308.90F, 5000.0F, "Basement Stairs"});
    moves.push_back(Move{-6241.24F, 7945.73F, 5000.0F, "Basement"});
    moves.push_back(Move{-5683.589844F, 12662.990234F, 5000.0F, "Lab Reaper"});
    moves.push_back(Move{-6035.58F, 11274.30F, 5000.0F, "Keeper 1/2"});
    moves.push_back(Move{-3881.71F, 11280.04F, 5000.0F, "Keeper 3"});
    moves.push_back(Move{-1502.45F, 9737.64F, 5000.0F, "Keeper 4/5"});
    moves.push_back(Move{-266.03F, 9304.26F, 5000.0F, "Lab Stairs1"});
    moves.push_back(Move{1207.05F, 7732.16F, 5000.0F, "Keeper 6"});
    moves.push_back(Move{1354.31F, 10063.58F, 5000.0F, "To Wastes 1"});
    moves.push_back(Move{3489.18F, 8177.49F, 5000.0F, "To Wastes 2"});
    moves.push_back(Move{5385.25F, 8866.17F, 5000.0F, "To Wastes 3"});
    moves.push_back(Move{6022.19F, 11318.40F, 5000.0F, "To Wastes 3"});
};

void EmoWindow::Draw(IDirect3DDevice9 *pDevice)
{
    UNREFERENCED_PARAMETER(pDevice);

    if (IsLoading())
        return;

    if (!visible)
        return;

    if (player.primary != GW::Constants::Profession::Elementalist ||
        player.secondary != GW::Constants::Profession::Monk)
        return;

    ImGui::SetNextWindowSize(ImVec2(110.0F, 330.0F), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("EmoWindow", nullptr, GetWinFlags()))
    {
        pumping.Draw();
        tank_bonding.Draw();
        player_bonding.Draw();
        fuse_pull.Draw();

        if (ImGui::Button(moves[move_idx].Name(), DEFAULT_BUTTON_SIZE))
        {
            moves[move_idx].Execute();
        }
        if (ImGui::Button("Prev.", ImVec2(DEFAULT_BUTTON_SIZE.x / 2.0F, DEFAULT_BUTTON_SIZE.y / 2.0F)))
        {
            if (move_idx > 0)
                --move_idx;
        }
        ImGui::SameLine();
        if (ImGui::Button("Next", ImVec2(DEFAULT_BUTTON_SIZE.x / 2.0F, DEFAULT_BUTTON_SIZE.y / 2.0F)))
        {
            if (move_idx < moves.size() - 1)
                ++move_idx;
        }
    }

    ImGui::End();
}

void EmoWindow::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);

    if (IsLoading() || IsOutpost())
        move_idx = 0;

    if (!IsUwEntryOutpost() && !IsUw())
        return;

    if (!player.ValidateData())
        return;
    player.Update();

    if (!skillbar.ValidateData())
        return;

    if (reset_template)
    {
        skillbar.Load();
        reset_template = false;
    }

    skillbar.Update();

    if (player.primary != GW::Constants::Profession::Elementalist ||
        player.secondary != GW::Constants::Profession::Monk)
        return;

    emo_casting_action_state = &pumping.action_state;

    if (send_move && (move_idx < moves.size() - 1) && GamePosCompare(player.pos, moves[move_idx].pos))
    {
        send_move = false;
        ++move_idx;
    }

    tank_bonding.Update();
    player_bonding.Update();
    fuse_pull.Update();
    pumping.Update();
}

RoutineState Pumping::Routine()
{
    static auto timer = TIMER_INIT();
    const auto timer_diff = TIMER_DIFF(timer);

    if (timer_diff < MIN_CYCLE_TIME_MS)
        return RoutineState::ACTIVE;

    timer = TIMER_INIT();

    if (!player->CanCast())
        return RoutineState::FINISHED;

    const bool found_balth = player->HasBuff(GW::Constants::SkillID::Balthazars_Spirit);
    const bool found_bond = player->HasBuff(GW::Constants::SkillID::Protective_Bond);

    const bool found_ether = player->HasEffect(GW::Constants::SkillID::Ether_Renewal);
    const bool found_sb = player->HasEffect(GW::Constants::SkillID::Spirit_Bond);
    const bool found_burning = player->HasEffect(GW::Constants::SkillID::Burning_Speed);

    if (skillbar->ether.CanBeCasted(player->energy))
    {
        (void)SafeUseSkill(skillbar->ether.idx, player->id);
        return RoutineState::ACTIVE;
    }

    const bool balth_avail = skillbar->balth.CanBeCasted(player->energy);
    if (!found_balth && balth_avail)
    {
        (void)SafeUseSkill(skillbar->balth.idx, player->id);
        return RoutineState::ACTIVE;
    }

    const bool bond_avail = skillbar->prot.CanBeCasted(player->energy);
    if (!found_bond && bond_avail)
    {
        (void)SafeUseSkill(skillbar->prot.idx, player->id);
        return RoutineState::ACTIVE;
    }

    const bool low_hp = player->hp_perc < 0.90F;
    const bool low_energy = player->energy_perc < 0.90F;

    const bool sb_needed = low_hp || !found_sb;
    const bool sb_avail = skillbar->sb.CanBeCasted(player->energy);
    if (found_ether && sb_needed && sb_avail)
    {
        (void)SafeUseSkill(skillbar->sb.idx, player->id);
        return RoutineState::ACTIVE;
    }

    const bool need_burning = low_hp || low_energy || !found_burning;
    const bool burning_avail = skillbar->burning.CanBeCasted(player->energy);
    if (found_ether && need_burning && burning_avail)
    {
        (void)SafeUseSkill(skillbar->burning.idx, player->id);
        return RoutineState::ACTIVE;
    }

#ifdef _DEBUG
    const auto is_in_dhuum_room = true;
#else
    const auto is_in_dhuum_room = IsInDhuumRoom(player);
#endif

    if (!is_in_dhuum_room)
        return RoutineState::FINISHED;

    if (found_turtle && turtle_id && GW::Agents::GetAgentByID(turtle_id))
    {
        const auto turtle_agent = GW::Agents::GetAgentByID(turtle_id);

        const auto dist = GW::GetDistance(player->pos, turtle_agent->pos);

        if (dist < GW::Constants::Range::Spellcast)
        {
            const auto found_bond = AgentHasBuff(GW::Constants::SkillID::Protective_Bond, turtle_agent->agent_id);
            const auto found_life = AgentHasBuff(GW::Constants::SkillID::Life_Bond, turtle_agent->agent_id);

            if (!found_bond)
            {
                (void)SafeUseSkill(skillbar->prot.idx, turtle_agent->agent_id);
                return RoutineState::ACTIVE;
            }

            if (!found_life)
            {
                (void)SafeUseSkill(skillbar->life.idx, turtle_agent->agent_id);
                return RoutineState::ACTIVE;
            }

            const auto turtle_living = turtle_agent->GetAsAgentLiving();

            if (turtle_living && turtle_living->hp < 0.7F)
            {
                (void)SafeUseSkill(skillbar->fuse.idx, turtle_agent->agent_id);
                return RoutineState::ACTIVE;
            }
            else if (turtle_living && turtle_living->hp < 0.9F)
            {
                (void)SafeUseSkill(skillbar->sb.idx, turtle_agent->agent_id);
                return RoutineState::ACTIVE;
            }
        }
    }

    uint32_t dhuum_id = 0;
#ifdef _DEBUG
    const auto is_in_dhuum_fight = true;
#else
    const auto is_in_dhuum_fight = IsInDhuumFight(&dhuum_id);
#endif

    if (!is_in_dhuum_fight)
        return RoutineState::FINISHED;

    if (skillbar->pi.SkillFound())
    {
        const auto dhuum_agent = GW::Agents::GetAgentByID(dhuum_id);

        if (dhuum_agent)
        {
            const auto dhuum_living = dhuum_agent->GetAsAgentLiving();

            if (dhuum_living && dhuum_living->GetIsCasting() && dhuum_living->skill == static_cast<uint32_t>(3085))
            {
                Log::Info("Dhuum is casting: %d", dhuum_living->skill);
                const bool pi_avail = skillbar->wisdom.CanBeCasted(player->energy);
                if (pi_avail)
                {
                    (void)SafeUseSkill(skillbar->pi.idx, dhuum_id);
                    return RoutineState::ACTIVE;
                }
            }
        }
    }

    if (skillbar->wisdom.SkillFound())
    {
        const bool wisdom_avail = skillbar->wisdom.CanBeCasted(player->energy);
        (void)SafeUseSkill(skillbar->wisdom.idx);
        return RoutineState::ACTIVE;
    }

    std::vector<PlayerMapping> party_members;
    bool success = GetPartyMembers(party_members);

    if (success && skillbar->gdw.SkillFound())
    {
        for (size_t idx = 0; idx < party_members.size(); idx++)
        {
            const auto id = party_members[idx].id;

            if (id == player->id)
                continue;

            const auto agent = GW::Agents::GetAgentByID(id);
            if (!agent)
                continue;

            const auto dist = GW::GetDistance(player->pos, agent->pos);
            if (dist > GW::Constants::Range::Spellcast)
                continue;

            if (PartyPlayerHasEffect(GW::Constants::SkillID::Great_Dwarf_Weapon, idx))
                continue;

            (void)SafeUseSkill(skillbar->gdw.idx, id);
            return RoutineState::ACTIVE;
        }
    }

    return RoutineState::FINISHED;
}

void Pumping::Update()
{
    static auto paused = false;
    static auto paused_by_reaper = false;

    if (player->living->GetIsMoving() && action_state == ActionState::ACTIVE)
    {
        action_state = ActionState::ON_HOLD;
        paused = true;
    }
    else if (player->target && action_state == ActionState::ACTIVE)
    {
        const auto dist = GW::GetDistance(player->pos, player->target->pos);

        const auto living_target = player->target->GetAsAgentLiving();

        if (living_target->player_number == static_cast<uint32_t>(GW::Constants::ModelID::UW::Reapers) &&
            dist < GW::Constants::Range::Earshot / 2.0F)
        {
            action_state = ActionState::ON_HOLD;
            paused = true;
            paused_by_reaper = true;
        }
    }

    if (action_state == ActionState::ACTIVE)
    {
        (void)(Routine());
    }

    if (paused && !player->living->GetIsMoving())
    {
        paused = false;
        action_state = ActionState::ACTIVE;
    }
    else if (paused && paused_by_reaper &&
             (!player->target ||
              player->target->agent_id != static_cast<uint32_t>(GW::Constants::ModelID::UW::Reapers)))
    {
        paused = false;
        paused_by_reaper = false;
        action_state = ActionState::ACTIVE;
    }

    if (GW::PartyMgr::GetIsPartyDefeated())
    {
        action_state = ActionState::INACTIVE;
    }
}

RoutineState TankBonding::Routine()
{
    static auto timer = TIMER_INIT();
    const auto timer_diff = TIMER_DIFF(timer);

    if (timer_diff < MIN_CYCLE_TIME_MS)
    {
        return RoutineState::ACTIVE;
    }

    timer = TIMER_INIT();

    if (!player->CanCast())
    {
        return RoutineState::ACTIVE;
    }

    // If no other player selected as target
    if ((!player->target || player->target->agent_id == player->id))
    {
        std::vector<PlayerMapping> party_members;

        bool success = GetPartyMembers(party_members);

        if (party_members.size() < 2)
        {
            return RoutineState::FINISHED;
        }

        uint32_t tank_idx = 0;

        switch (GW::Map::GetMapID())
        {
        case GW::Constants::MapID::The_Underworld:
#ifdef _DEBUG
        case GW::Constants::MapID::Isle_of_the_Nameless:
#endif
        {
            tank_idx = party_members.size() - 2;
            break;
        }
        default:
        {
            tank_idx = 0;
            break;
        }
        }

        const auto tank = party_members[tank_idx];
        player->ChangeTarget(tank.id);

        if (!success || !player->target)
        {
            return RoutineState::FINISHED;
        }
    }

    if (player->target->type != static_cast<uint32_t>(GW::Constants::AgentType::Living))
    {
        return RoutineState::FINISHED;
    }

    const auto target_living = player->target->GetAsAgentLiving();

    if (target_living->allegiance != static_cast<uint8_t>(GW::Constants::Allegiance::Ally_NonAttackable) ||
        target_living->GetIsDead())
    {
        return RoutineState::FINISHED;
    }

    const bool found_balth = AgentHasBuff(GW::Constants::SkillID::Balthazars_Spirit, player->target->agent_id);
    const bool found_bond = AgentHasBuff(GW::Constants::SkillID::Protective_Bond, player->target->agent_id);
    const bool found_life = AgentHasBuff(GW::Constants::SkillID::Life_Bond, player->target->agent_id);

    if (!found_balth && skillbar->balth.CanBeCasted(player->energy))
    {
        (void)SafeUseSkill(skillbar->balth.idx, player->target->agent_id);
        return RoutineState::ACTIVE;
    }

    if (!found_bond && skillbar->prot.CanBeCasted(player->energy))
    {
        (void)SafeUseSkill(skillbar->prot.idx, player->target->agent_id);
        return RoutineState::ACTIVE;
    }

    if (!found_life && skillbar->life.CanBeCasted(player->energy))
    {
        (void)SafeUseSkill(skillbar->life.idx, player->target->agent_id);
        return RoutineState::ACTIVE;
    }

    return RoutineState::FINISHED;
}

void TankBonding::Update()
{
    if (action_state == ActionState::ACTIVE)
    {
        StateOnHold(*emo_casting_action_state);
        const auto routine_state = Routine();

        if (routine_state == RoutineState::FINISHED)
        {
            action_state = ActionState::INACTIVE;
            StateOnActive(*emo_casting_action_state);
        }

        if (GW::PartyMgr::GetIsPartyDefeated())
        {
            action_state = ActionState::INACTIVE;
        }
    }
}

RoutineState PlayerBonding::Routine()
{
    static auto timer = TIMER_INIT();
    const auto timer_diff = TIMER_DIFF(timer);

    if (timer_diff < MIN_CYCLE_TIME_MS)
    {
        return RoutineState::ACTIVE;
    }

    timer = TIMER_INIT();

    if (!player->CanCast())
    {
        return RoutineState::ACTIVE;
    }

    if (!player->target || player->target->type != static_cast<uint32_t>(GW::Constants::AgentType::Living))
    {
        return RoutineState::FINISHED;
    }

    const auto target_living = player->target->GetAsAgentLiving();

    if (target_living->allegiance != static_cast<uint8_t>(GW::Constants::Allegiance::Ally_NonAttackable) ||
        target_living->GetIsDead())
    {
        return RoutineState::FINISHED;
    }

    const bool found_balth = AgentHasBuff(GW::Constants::SkillID::Balthazars_Spirit, player->target->agent_id);
    const bool found_bond = AgentHasBuff(GW::Constants::SkillID::Protective_Bond, player->target->agent_id);

    if (!found_balth && skillbar->balth.CanBeCasted(player->energy))
    {
        (void)SafeUseSkill(skillbar->balth.idx, player->target->agent_id);
        return RoutineState::ACTIVE;
    }

    if (!found_bond && skillbar->prot.CanBeCasted(player->energy))
    {
        (void)SafeUseSkill(skillbar->prot.idx, player->target->agent_id);
        return RoutineState::ACTIVE;
    }

    return RoutineState::FINISHED;
}

void PlayerBonding::Update()
{
    if (action_state == ActionState::ACTIVE)
    {
        StateOnHold(*emo_casting_action_state);
        const auto routine_state = Routine();

        if (routine_state == RoutineState::FINISHED)
        {
            action_state = ActionState::INACTIVE;
            StateOnActive(*emo_casting_action_state);
        }

        if (GW::PartyMgr::GetIsPartyDefeated())
        {
            action_state = ActionState::INACTIVE;
        }
    }
}

RoutineState FusePull::Routine()
{
    ResetState(routine_state);

    constexpr auto cancel_step1 = 0;
    constexpr auto cancel_step2 = 1;
    constexpr auto fuse_step = 2;

    if (!player->target || player->target->type != static_cast<uint32_t>(GW::Constants::AgentType::Living))
    {
        ResetData();
        return RoutineState::FINISHED;
    }

    const auto target_living = player->target->GetAsAgentLiving();

    if (target_living->allegiance != static_cast<uint8_t>(GW::Constants::Allegiance::Ally_NonAttackable) ||
        target_living->GetIsDead())
    {
        ResetData();
        return RoutineState::FINISHED;
    }

    const auto timer_diff = TIMER_DIFF(timer);

    if ((routine_state == RoutineState::NONE) && (step == cancel_step1))
    {
        GW::CtoS::SendPacket(0x4, GAME_CMSG_CANCEL_MOVEMENT);
        ++step;

        return RoutineState::ACTIVE;
    }
    if (step == cancel_step1 + 1 && timer_diff > 300)
    {
        timer = TIMER_INIT();
    }
    else if (step == cancel_step1 + 1 && timer_diff < 300)
    {
        return RoutineState::ACTIVE;
    }

    const auto me_pos = player->pos;
    const auto target_pos = player->target->pos;

    const auto d = GW::GetDistance(me_pos, target_pos);

    if ((routine_state == RoutineState::NONE) && (d < MIN_FUSE_PULL_RANGE || d > MAX_FUSE_PULL_RANGE))
    {
        requested_pos = GW::GamePos{};

        const auto m_x = me_pos.x;
        const auto m_y = me_pos.y;
        const auto t_x = target_pos.x;
        const auto t_y = target_pos.y;

        const auto d_t = FUSE_PULL_RANGE;
        const auto t = d_t / d;

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

    if (step == cancel_step2)
    {
        GW::CtoS::SendPacket(0x4, GAME_CMSG_CANCEL_MOVEMENT);
        ++step;

        return RoutineState::ACTIVE;
    }
    if (step == cancel_step2 + 1 && timer_diff > 300)
    {
        timer = TIMER_INIT();
    }
    else if (step == cancel_step2 + 1 && timer_diff < 300)
    {
        return RoutineState::ACTIVE;
    }

    if (step == fuse_step)
    {
        if (!player->CanCast())
        {
            return RoutineState::ACTIVE;
        }

        if (skillbar->fuse.CanBeCasted(player->energy))
        {
            (void)SafeUseSkill(skillbar->fuse.idx, player->target->agent_id);
        }
        ++step;
        return RoutineState::ACTIVE;
    }

    ResetData();
    return RoutineState::FINISHED;
}

void FusePull::Update()
{
    if (action_state == ActionState::ACTIVE)
    {
        StateOnHold(*emo_casting_action_state);
        const auto routine_state = Routine();

        if (routine_state == RoutineState::FINISHED)
        {
            action_state = ActionState::INACTIVE;
            StateOnActive(*emo_casting_action_state);
        }

        if (GW::PartyMgr::GetIsPartyDefeated())
        {
            action_state = ActionState::INACTIVE;
        }
    }
}

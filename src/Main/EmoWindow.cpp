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
static const auto DEFAULT_WINDOW_SIZE = ImVec2(120.0, DEFAULT_BUTTON_SIZE.x * 3.0F);
static ActionState *emo_casting_action_state = nullptr;
static constexpr auto MIN_CYCLE_TIME_MS = uint32_t{100};
}; // namespace


void ActionABC::Draw(const ImVec2 button_size)
{
    const auto color = COLOR_MAPPING[static_cast<uint32_t>(action_state)];
    DrawButton(action_state, color, text, button_size);
}

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

    ImGui::SetNextWindowSize(DEFAULT_WINDOW_SIZE, ImGuiCond_FirstUseEver);

    if (ImGui::Begin("EmoWindow", nullptr, GetWinFlags()))
    {
        pumping.Draw();
        tank_bonding.Draw();
        player_bonding.Draw();
        fuse_pull.Draw();
    }

    ImGui::End();
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

    const float moving_flag = static_cast<float>(player->living->GetIsMoving());
    const float moving_offset_hp = moving_flag * 0.10F;
    const float moving_offset_energy = moving_flag * 0.20F;

    const bool low_hp = player->hp_perc < (0.90F - moving_offset_hp);
    const bool low_energy = player->energy_perc < (0.90F - moving_offset_energy);

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

    return RoutineState::FINISHED;
}

void Pumping::Update()
{
    static auto paused = false;

    if (player->living->GetIsMoving() && action_state == ActionState::ACTIVE)
    {
        action_state = ActionState::ON_HOLD;
        paused = true;
    }
    else if (action_state == ActionState::ACTIVE)
    {
        (void)(Routine());
    }

    if (paused && !player->living->GetIsMoving())
    {
        paused = false;
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
        case GW::Constants::MapID::Isle_of_the_Nameless: /* Debug area ;) */
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

    const auto me_pos = player->pos;
    const auto target_pos = player->target->pos;

    const auto d = GW::GetDistance(me_pos, target_pos);

    if ((routine_state == RoutineState::NONE) && (d < MIN_FUSE_PULL_RANGE || d > MAX_FUSE_PULL_RANGE))
    {
        GW::CtoS::SendPacket(0x4, GAME_CMSG_CANCEL_MOVEMENT);

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

    constexpr auto cancel_step = 0;
    constexpr auto fuse_step = 1;

    constexpr auto time_wait_ms = 600;

    const auto timer_diff = TIMER_DIFF(timer);

    if (step == cancel_step)
    {
        GW::CtoS::SendPacket(0x4, GAME_CMSG_CANCEL_MOVEMENT);
        ++step;

        return RoutineState::ACTIVE;
    }
    if (step == cancel_step + 1 && timer_diff > time_wait_ms)
    {
        timer = TIMER_INIT();
    }
    else if (step == cancel_step + 1 && timer_diff < time_wait_ms)
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
    if (step == fuse_step + 1 && timer_diff > time_wait_ms + 200)
    {
        timer = TIMER_INIT();
    }
    else if (step == fuse_step + 1 && timer_diff < time_wait_ms + 200)
    {
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

void EmoWindow::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);

    if (!player.ValidateData())
        return;
    player.Update();

    if (!skillbar.ValidateData())
        return;
    skillbar.Update();

    if (player.primary != GW::Constants::Profession::Elementalist ||
        player.secondary != GW::Constants::Profession::Monk)
        return;

    emo_casting_action_state = &pumping.action_state;

    tank_bonding.Update();
    player_bonding.Update();
    fuse_pull.Update();
    pumping.Update();
}

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
#include <Helper.h>
#include <Player.h>
#include <Skillbars.h>
#include <Types.h>
#include <Utils.h>

#include "EmoWindow.h"

namespace
{
static ActionState *emo_casting_action_state = nullptr;

static const auto WINDOW_SIZE = ImVec2(120.0, 370.0);
static const auto BUTTON_SIZE = ImVec2(120.0, 80.0);

static const auto ACTIVE_COLOR = ImVec4{0.0F, 200.0F, 0.0F, 80.0F};
static const auto INACTIVE_COLOR = ImVec4{41.0F, 74.0F, 122.0F, 80.0F};
static const auto ON_HOLD_COLOR = ImVec4{255.0F, 226.0F, 0.0F, 80.0F};

static constexpr auto MIN_CYCLE_TIME_MS = uint32_t{100};

static auto COLOR_MAPPING = std::map<uint32_t, ImVec4>{{static_cast<uint32_t>(ActionState::INACTIVE), INACTIVE_COLOR},
                                                       {static_cast<uint32_t>(ActionState::ACTIVE), ACTIVE_COLOR},
                                                       {static_cast<uint32_t>(ActionState::ON_HOLD), ON_HOLD_COLOR}};
}; // namespace

void DrawButton(ActionState &action_state, const ImVec4 color, std::string_view text)
{
    bool pushed_style = false;

    if (action_state != ActionState::INACTIVE)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        pushed_style = true;
    }

    if (ImGui::Button(text.data(), BUTTON_SIZE))
    {
        if (IsExplorable())
        {
            action_state = StateNegation(action_state);
        }
    }
    if (pushed_style)
        ImGui::PopStyleColor();
}

void EmoAction::Draw()
{
    const auto color = COLOR_MAPPING[static_cast<uint32_t>(action_state)];
    DrawButton(action_state, color, text);
}

void EmoWindow::Draw(IDirect3DDevice9 *pDevice)
{
    UNREFERENCED_PARAMETER(pDevice);

    if (GW::Map::GetInstanceType() == GW::Constants::InstanceType::Loading)
        return;

    if (!visible)
        return;

    ImGui::SetNextWindowSize(WINDOW_SIZE, ImGuiCond_FirstUseEver);

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
    {
        return RoutineState::ACTIVE;
    }

    timer = TIMER_INIT();

    if (!player->CanCast())
    {
        return RoutineState::FINISHED;
    }

    const bool found_balth = player->HasBuff(GW::Constants::SkillID::Balthazars_Spirit);
    const bool found_bond = player->HasBuff(GW::Constants::SkillID::Protective_Bond);

    const bool found_ether = player->HasEffect(GW::Constants::SkillID::Ether_Renewal);
    const bool found_sb = player->HasEffect(GW::Constants::SkillID::Spirit_Bond);
    const bool found_burning = player->HasEffect(GW::Constants::SkillID::Burning_Speed);

    if (player->skillbar.ether.CanBeCasted(player->energy))
    {
        SafeUseSkill(player->skillbar.ether.idx, player->id);
        return RoutineState::ACTIVE;
    }

    const bool balth_avail = player->skillbar.balth.CanBeCasted(player->energy);
    if (!found_balth && balth_avail)
    {
        SafeUseSkill(player->skillbar.balth.idx, player->id);
        return RoutineState::ACTIVE;
    }

    const bool bond_avail = player->skillbar.bond.CanBeCasted(player->energy);
    if (!found_bond && bond_avail)
    {
        SafeUseSkill(player->skillbar.bond.idx, player->id);
        return RoutineState::ACTIVE;
    }

    const float moving_flag = static_cast<float>(player->living->GetIsMoving());
    const float moving_offset_hp = moving_flag * 0.10F;
    const float moving_offset_energy = moving_flag * 0.20F;

    const bool low_hp = player->hp_perc < (0.90F - moving_offset_hp);
    const bool low_energy = player->energy_perc < (0.90F - moving_offset_energy);

    const bool sb_needed = low_hp || !found_sb;
    const bool sb_avail = player->skillbar.sb.CanBeCasted(player->energy);
    if (found_ether && sb_needed && sb_avail)
    {
        SafeUseSkill(player->skillbar.sb.idx, player->id);
        return RoutineState::ACTIVE;
    }

    const bool need_burning = low_hp || low_energy || !found_burning;
    const bool burning_avail = player->skillbar.burning.CanBeCasted(player->energy);
    if (found_ether && need_burning && burning_avail)
    {
        SafeUseSkill(player->skillbar.burning.idx, player->id);
        return RoutineState::ACTIVE;
    }

    return RoutineState::FINISHED;
}

void Pumping::Update()
{
    emo_casting_action_state = &action_state;

    if (action_state == ActionState::ACTIVE)
    {
        static_cast<void>(Routine());
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

    if (player->target->type != 0xDB)
    {
        return RoutineState::FINISHED;
    }

    const auto target_living = player->target->GetAsAgentLiving();

    if (target_living->allegiance != 0x1 || target_living->GetIsDead())
    {
        return RoutineState::FINISHED;
    }

    const bool found_balth = AgentHasBuff(GW::Constants::SkillID::Balthazars_Spirit, player->target->agent_id);
    const bool found_bond = AgentHasBuff(GW::Constants::SkillID::Protective_Bond, player->target->agent_id);
    const bool found_life = AgentHasBuff(GW::Constants::SkillID::Life_Bond, player->target->agent_id);

    if (!found_balth && player->skillbar.balth.CanBeCasted(player->energy))
    {
        SafeUseSkill(player->skillbar.balth.idx, player->target->agent_id);
        return RoutineState::ACTIVE;
    }

    if (!found_bond && player->skillbar.bond.CanBeCasted(player->energy))
    {
        SafeUseSkill(player->skillbar.bond.idx, player->target->agent_id);
        return RoutineState::ACTIVE;
    }

    if (!found_life && player->skillbar.life.CanBeCasted(player->energy))
    {
        SafeUseSkill(player->skillbar.life.idx, player->target->agent_id);
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

    if (!player->target || player->target->type != 0xDB)
    {
        return RoutineState::FINISHED;
    }

    const auto target_living = player->target->GetAsAgentLiving();

    if (target_living->allegiance != 0x1 || target_living->GetIsDead())
    {
        return RoutineState::FINISHED;
    }

    const bool found_balth = AgentHasBuff(GW::Constants::SkillID::Balthazars_Spirit, player->target->agent_id);
    const bool found_bond = AgentHasBuff(GW::Constants::SkillID::Protective_Bond, player->target->agent_id);

    if (!found_balth && player->skillbar.balth.CanBeCasted(player->energy))
    {
        SafeUseSkill(player->skillbar.balth.idx, player->target->agent_id);
        return RoutineState::ACTIVE;
    }

    if (!found_bond && player->skillbar.bond.CanBeCasted(player->energy))
    {
        SafeUseSkill(player->skillbar.bond.idx, player->target->agent_id);
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
    }
}

RoutineState FusePull::Routine()
{
    ResetState(routine_state);

    if (!player->target || player->target->type != 0xDB)
    {
        ResetData();
        return RoutineState::FINISHED;
    }

    const auto target_living = player->target->GetAsAgentLiving();

    if (target_living->allegiance != 0x1 || target_living->GetIsDead())
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

    constexpr auto cancel1_step = 0;
    constexpr auto armor1_step = 1;
    constexpr auto fuse_step = 2;
    constexpr auto cancel2_step = 3;
    constexpr auto armor2_step = 4;
    constexpr auto time_wait_ms = 600;
    constexpr auto bag_idx = 5;
    constexpr auto starting_slid_idx = 16;

    const auto timer_diff = TIMER_DIFF(timer);

    if (step == cancel1_step)
    {
        GW::CtoS::SendPacket(0x4, GAME_CMSG_CANCEL_MOVEMENT);
        ++step;

        return RoutineState::ACTIVE;
    }
    if (step == cancel1_step + 1 && timer_diff > time_wait_ms)
    {
        timer = TIMER_INIT();
    }
    else if (step == cancel1_step + 1 && timer_diff < time_wait_ms)
    {
        return RoutineState::ACTIVE;
    }

    if (step == armor1_step)
    {
        ChangeFullArmor(bag_idx, starting_slid_idx);
        ++step;
        return RoutineState::ACTIVE;
    }
    if (step == armor1_step + 1 && timer_diff > time_wait_ms)
    {
        timer = TIMER_INIT();
    }
    else if (step == armor1_step + 1 && timer_diff < time_wait_ms)
    {
        return RoutineState::ACTIVE;
    }

    if (step == fuse_step)
    {
        if (!player->CanCast())
        {
            return RoutineState::ACTIVE;
        }

        if (player->skillbar.fuse.CanBeCasted(player->energy))
        {
            SafeUseSkill(player->skillbar.fuse.idx, player->target->agent_id);
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

    if (step == cancel2_step)
    {
        GW::CtoS::SendPacket(0x4, GAME_CMSG_CANCEL_MOVEMENT);
        ++step;

        return RoutineState::ACTIVE;
    }
    if (step == cancel2_step + 1 && timer_diff > time_wait_ms)
    {
        timer = TIMER_INIT();
    }
    else if (step == cancel2_step + 1 && timer_diff < time_wait_ms)
    {
        return RoutineState::ACTIVE;
    }

    if (step == armor2_step)
    {
        ChangeFullArmor(bag_idx, starting_slid_idx);
        ++step;

        return RoutineState::ACTIVE;
    }
    if (step == armor2_step + 1 && timer_diff > time_wait_ms)
    {
        timer = TIMER_INIT();
    }
    else if (step == armor2_step + 1 && timer_diff < time_wait_ms)
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
    }
}

void EmoWindow::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);

    if (!player.ValidateData())
    {
        return;
    }

    player.Update();

    tank_bonding.Update();
    player_bonding.Update();
    fuse_pull.Update();
    pumping.Update();
}

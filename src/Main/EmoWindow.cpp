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
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Packets/Opcodes.h>

#include <fmt/format.h>

#include <GuiUtils.h>
#include <HelperBox.h>
#include <Logger.h>

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
static ActionState *emo_casting_action_state = nullptr;
static bool send_move = false;
}; // namespace

void ActionABC::Draw(const ImVec2 button_size)
{
    if (!IsExplorable())
        action_state = ActionState::INACTIVE;

    const auto color = COLOR_MAPPING[static_cast<uint32_t>(action_state)];
    DrawButton(action_state, color, text, button_size);
}

EmoWindow::EmoWindow()
    : player({}), skillbar({}), fuse_pull(&player, &skillbar), pumping(&player, &skillbar),
      tank_bonding(&player, &skillbar), player_bonding(&player, &skillbar)
{
    if (skillbar.ValidateData())
        skillbar.Load();
};

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
            if (ImGui::Button(moves[move_idx].Name(), DEFAULT_BUTTON_SIZE))
            {
                moves[move_idx].Execute();
                send_move = true;
            }
            if (ImGui::Button("Prev.", ImVec2(DEFAULT_BUTTON_SIZE.x / 2.25F, DEFAULT_BUTTON_SIZE.y / 2.0F)))
            {
                if (move_idx > 0)
                    --move_idx;
            }
            ImGui::SameLine();
            if (ImGui::Button("Next", ImVec2(DEFAULT_BUTTON_SIZE.x / 2.25F, DEFAULT_BUTTON_SIZE.y / 2.0F)))
            {
                if (move_idx < moves.size() - 1)
                    ++move_idx;
            }
        }
    }

    ImGui::End();
}

void EmoWindow::Update(float delta)
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

    emo_casting_action_state = &pumping.action_state;

    if (IsUw() && send_move)
    {
        if ((move_idx < moves.size() - 1U) && GamePosCompare(player.pos, moves[move_idx].pos))
        {
            send_move = false;
            ++move_idx;
        }
    }

    tank_bonding.Update();
    player_bonding.Update();
    fuse_pull.Update();
    pumping.Update();
}

bool EmoWindow::ActivationConditions()
{
    if (player.primary == GW::Constants::Profession::Elementalist &&
        player.secondary == GW::Constants::Profession::Monk)
        return true;

    return false;
}

Pumping::Pumping(Player *p, EmoSkillbar *s) : EmoActionABC(p, "Pumping", s)
{
    GW::StoC::RegisterPacketCallback<GW::Packet::StoC::AgentAdd>(
        &Summon_AgentAdd_Entry,
        [&](GW::HookStatus *, GW::Packet::StoC::AgentAdd *pak) -> void {
            if (pak->type != 1)
                return;

            uint32_t player_number = (pak->agent_type ^ 0x20000000);

            if (!IsUw() || player_number != 514) // Turtle id
                return;

            found_turtle = true;
            turtle_id = pak->agent_id;
        });

    GW::StoC::RegisterPacketCallback<GW::Packet::StoC::GenericValue>(
        &GenericValue_Entry,
        [this](GW::HookStatus *status, GW::Packet::StoC::GenericValue *packet) -> void {
            UNREFERENCED_PARAMETER(status);
            if (action_state == ActionState::ACTIVE && SkillStoppedCallback(packet, player))
                interrupted = true;
        });
}

RoutineState Pumping::RoutineSelfBonds()
{
    const auto found_ether = player->HasEffect(GW::Constants::SkillID::Ether_Renewal);
    if (skillbar->ether.CanBeCasted(player->energy))
        return SafeUseSkill(skillbar->ether.idx, player->id);

    if (player->energy_perc > 0.5)
    {
        const auto found_balth = player->HasBuff(GW::Constants::SkillID::Balthazars_Spirit);
        const auto balth_avail = skillbar->balth.CanBeCasted(player->energy);
        if (!found_balth && balth_avail)
            return SafeUseSkill(skillbar->balth.idx, player->id);

        const auto found_bond = player->HasBuff(GW::Constants::SkillID::Protective_Bond);
        const auto bond_avail = skillbar->prot.CanBeCasted(player->energy);
        if (!found_bond && bond_avail)
            return SafeUseSkill(skillbar->prot.idx, player->id);
    }

    const auto found_sb = player->HasEffect(GW::Constants::SkillID::Spirit_Bond);
    const auto low_hp = player->hp_perc < 0.90F;

    const auto sb_needed = low_hp || !found_sb;
    const auto sb_avail = skillbar->sb.CanBeCasted(player->energy);
    if (found_ether && sb_needed && sb_avail)
        return SafeUseSkill(skillbar->sb.idx, player->id);

    const auto found_burning = player->HasEffect(GW::Constants::SkillID::Burning_Speed);
    const auto low_energy = player->energy_perc < 0.90F;

    const auto need_burning = low_hp || low_energy || !found_burning;
    const auto burning_avail = skillbar->burning.CanBeCasted(player->energy);
    if (found_ether && need_burning && burning_avail)
        return SafeUseSkill(skillbar->burning.idx, player->id);

    return RoutineState::ACTIVE;
}

RoutineState Pumping::RoutineLT()
{
    if (!player->target || player->living->GetIsMoving())
        return RoutineState::ACTIVE;

    const auto target_living = player->target->GetAsAgentLiving();
    if (!target_living || target_living->GetIsMoving() ||
        target_living->primary != static_cast<uint8_t>(GW::Constants::Profession::Mesmer))
        return RoutineState::ACTIVE;

    const auto dist = GW::GetDistance(player->pos, player->target->pos);

    if (dist < FuseRange::FUSE_PULL_RANGE - 1.0F || dist > FuseRange::FUSE_PULL_RANGE + 1.0F)
        return RoutineState::ACTIVE;

    const uint32_t player_idx = GetPartyIdxByID(target_living->agent_id);

    const auto found_sb = PartyPlayerHasEffect(skillbar->sb.id, player_idx);
    if (!found_sb && skillbar->sb.CanBeCasted(player->energy))
        return SafeUseSkill(skillbar->sb.idx, target_living->agent_id);

    return RoutineState::ACTIVE;
}

RoutineState Pumping::RoutineTurtle()
{
    if (!found_turtle || !turtle_id)
        return RoutineState::ACTIVE;

    const auto turtle_agent = GW::Agents::GetAgentByID(turtle_id);
    if (!turtle_agent)
        return RoutineState::ACTIVE;

    const auto dist = GW::GetDistance(player->pos, turtle_agent->pos);

    if (dist > GW::Constants::Range::Spellcast)
        return RoutineState::ACTIVE;

    const auto found_bond = AgentHasBuff(GW::Constants::SkillID::Protective_Bond, turtle_agent->agent_id);
    if (!found_bond)
        return SafeUseSkill(skillbar->prot.idx, turtle_agent->agent_id);

    const auto found_life = AgentHasBuff(GW::Constants::SkillID::Life_Bond, turtle_agent->agent_id);
    if (!found_life)
        return SafeUseSkill(skillbar->life.idx, turtle_agent->agent_id);

    const auto turtle_living = turtle_agent->GetAsAgentLiving();
    if (turtle_living && turtle_living->hp < 0.7F)
        return SafeUseSkill(skillbar->fuse.idx, turtle_agent->agent_id);
    else if (turtle_living && turtle_living->hp < 0.9F)
        return SafeUseSkill(skillbar->sb.idx, turtle_agent->agent_id);

    return RoutineState::ACTIVE;
}

RoutineState Pumping::RoutinePI(const uint32_t dhuum_id)
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
    {
        const auto pi_avail = skill.CanBeCasted(player->energy);
        if (pi_avail)
            return SafeUseSkill(skill.idx, dhuum_id);
    }

    return RoutineState::ACTIVE;
}

RoutineState Pumping::RoutineWisdom()
{
    const auto &skill = skillbar->wisdom;

    if (!skill.SkillFound())
        return RoutineState::ACTIVE;

    const auto wisdom_avail = skill.CanBeCasted(player->energy);
    if (wisdom_avail)
        return SafeUseSkill(skill.idx);

    return RoutineState::ACTIVE;
}

RoutineState Pumping::RoutineGDW()
{
    const auto &skill = skillbar->gdw;

    if (!skill.SkillFound())
        return RoutineState::ACTIVE;

    std::vector<PlayerMapping> party_members;
    const auto success = GetPartyMembers(party_members);

    if (!success)
        return RoutineState::ACTIVE;

    for (size_t idx = 0; idx < party_members.size(); idx++)
    {
        const auto id = party_members[idx].id;

        if (id == player->id)
            continue;

        const auto agent = GW::Agents::GetAgentByID(id);
        if (!agent)
            continue;

        const auto living = agent->GetAsAgentLiving();
        if (!living || living->GetIsMoving())
            continue;

        const auto dist = GW::GetDistance(player->pos, agent->pos);
        if (dist > GW::Constants::Range::Spellcast)
            continue;

        if (PartyPlayerHasEffect(skill.id, idx))
            continue;

        return SafeUseSkill(skill.idx, id);
    }

    return RoutineState::ACTIVE;
}

RoutineState Pumping::Routine()
{
    if (!player->CanCast())
        return RoutineState::FINISHED;

    const auto self_bonds_state = RoutineSelfBonds();
    if (self_bonds_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    if (!IsUw())
        return RoutineState::FINISHED;

    const auto lt_state = RoutineLT();
    if (lt_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    const auto is_in_dhuum_room = IsInDhuumRoom(player);
    if (!is_in_dhuum_room)
        return RoutineState::FINISHED;

    const auto turtle_state = RoutineTurtle();
    if (turtle_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    uint32_t dhuum_id = 0;
    const auto is_in_dhuum_fight = IsInDhuumFight(&dhuum_id);

    if (!is_in_dhuum_fight || !dhuum_id)
        return RoutineState::FINISHED;

    const auto wisdom_state = RoutineWisdom();
    if (wisdom_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    const auto gdw_state = RoutineGDW();
    if (gdw_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    const auto pi_state = RoutinePI(dhuum_id);
    if (pi_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    return RoutineState::FINISHED;
}

void Pumping::Update()
{
    static auto paused = false;
    static auto paused_by_reaper = false;

    if (IsUw())
    {
        const uint32_t lt_id = GetTankId();
        lt_agent = GW::Agents::GetAgentByID(lt_id);
    }

    const auto keyboard_move_request =
        ImGui::IsKeyPressed(87) || ImGui::IsKeyPressed(68) || ImGui::IsKeyPressed(65) || ImGui::IsKeyPressed(83);
    if ((player->living->GetIsMoving() || (ImGui::IsKeyPressed(VK_CONTROL) && keyboard_move_request)) &&
        action_state == ActionState::ACTIVE)
    {
        action_state = ActionState::ON_HOLD;
        paused = true;
    }
    else if (player->target && action_state == ActionState::ACTIVE)
    {
        const auto dist = GW::GetDistance(player->pos, player->target->pos);

        const auto living_target = player->target->GetAsAgentLiving();

        if (living_target &&
            (living_target->player_number == static_cast<uint32_t>(GW::Constants::ModelID::UW::Reapers)) &&
            (dist < GW::Constants::Range::Earshot / 2.0F))
        {
            action_state = ActionState::ON_HOLD;
            paused = true;
            paused_by_reaper = true;
        }
    }

    if (action_state == ActionState::ACTIVE)
        (void)(Routine());

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
        action_state = ActionState::INACTIVE;

    if (interrupted)
    {
        interrupted = false;
        action_state = ActionState::INACTIVE;
    }
}

RoutineState TankBonding::Routine()
{
    static uint32_t target_id = 0;

    if (!player->CanCast())
        return RoutineState::ACTIVE;

    if (interrupted)
    {
        interrupted = false;
        return RoutineState::FINISHED;
    }

    // If no other player selected as target
    const auto no_target_or_self = (!player->target || player->target->agent_id == player->id);
    const auto target_not_self = (player->target && player->target->agent_id != player->id);

    // Get target at activation, afrter keep the id
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

    const auto found_balth = AgentHasBuff(GW::Constants::SkillID::Balthazars_Spirit, target_id);
    const auto found_bond = AgentHasBuff(GW::Constants::SkillID::Protective_Bond, target_id);
    const auto found_life = AgentHasBuff(GW::Constants::SkillID::Life_Bond, target_id);

    if (!found_balth && skillbar->balth.CanBeCasted(player->energy))
    {
        (void)SafeUseSkill(skillbar->balth.idx, target_id);
        return RoutineState::ACTIVE;
    }

    if (!found_bond && skillbar->prot.CanBeCasted(player->energy))
    {
        (void)SafeUseSkill(skillbar->prot.idx, target_id);
        return RoutineState::ACTIVE;
    }

    if (!found_life && skillbar->life.CanBeCasted(player->energy))
    {
        (void)SafeUseSkill(skillbar->life.idx, target_id);
        return RoutineState::ACTIVE;
    }

    target_id = 0;
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
            action_state = ActionState::INACTIVE;
    }
}

RoutineState PlayerBonding::Routine()
{
    static uint32_t target_id = 0;

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

    const auto found_balth = AgentHasBuff(GW::Constants::SkillID::Balthazars_Spirit, target_id);
    if (!found_balth && skillbar->balth.CanBeCasted(player->energy))
    {
        (void)SafeUseSkill(skillbar->balth.idx, target_id);
        return RoutineState::ACTIVE;
    }

    const auto found_bond = AgentHasBuff(GW::Constants::SkillID::Protective_Bond, target_id);
    if (!found_bond && skillbar->prot.CanBeCasted(player->energy))
    {
        (void)SafeUseSkill(skillbar->prot.idx, target_id);
        return RoutineState::ACTIVE;
    }

    target_id = 0;
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
            action_state = ActionState::INACTIVE;
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
        requested_pos = GW::GamePos{};

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
    if (action_state == ActionState::ACTIVE)
    {
        StateOnHold(*emo_casting_action_state);
        routine_state = Routine();

        if (routine_state == RoutineState::FINISHED)
        {
            action_state = ActionState::INACTIVE;
            StateOnActive(*emo_casting_action_state);
        }

        if (GW::PartyMgr::GetIsPartyDefeated())
            action_state = ActionState::INACTIVE;
    }
}

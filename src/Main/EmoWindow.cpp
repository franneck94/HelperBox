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

#include <Actions.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <MathUtils.h>
#include <Player.h>
#include <Skillbars.h>
#include <Types.h>
#include <UwHelper.h>

#include "EmoWindow.h"

namespace
{
static ActionState *emo_casting_action_state = nullptr;
static bool send_move = false;

const auto SKIP_BUTTON_X = DEFAULT_BUTTON_SIZE.x / 2.25F;
const auto SKIP_BUTTON_Y = DEFAULT_BUTTON_SIZE.y / 2.0F;
const auto SKIP_BUTTON_SIZE = ImVec2(SKIP_BUTTON_X, SKIP_BUTTON_Y);
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
            load_cb_triggered = MapLoadCallback(status, packet);
        });
};

void EmoWindow::WarnDistanceLT() const
{
    static auto warned = false;
    constexpr auto warn_dist = GW::Constants::Range::Compass - 20.0F;

    const uint32_t lt_id = GetTankId();
    const auto lt_agent = GW::Agents::GetAgentByID(lt_id);

    if (!lt_agent)
        return;

    const auto dist = GW::GetDistance(player.pos, lt_agent->pos);
    if (!warned && dist > warn_dist)
    {
        GW::Chat::WriteChat(GW::Chat::Channel::CHANNEL_GLOBAL, L"LT is leaving EMO's compass range!");
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
            if (ImGui::Button(moves[move_idx].Name(), DEFAULT_BUTTON_SIZE))
            {
                moves[move_idx].Execute();
                send_move = true;
            }
            if (ImGui::Button("Prev.", SKIP_BUTTON_SIZE))
            {
                if (move_idx > 0)
                    --move_idx;
            }
            ImGui::SameLine();
            if (ImGui::Button("Next", SKIP_BUTTON_SIZE))
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

    static auto triggered_tank_bonds_at_start = false;
    static auto triggered_move_start = false;

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
        if ((move_idx < moves.size() - 1U) && GamePosCompare(player.pos, moves[move_idx].pos, 0.1F))
        {
            send_move = false;
            ++move_idx;
        }
    }

    // START Automated Actions at UW Entry
    if (IsUw())
    {
        if (load_cb_triggered)
        {
            tank_bonding.action_state = ActionState::ACTIVE;
            load_cb_triggered = false;
            triggered_tank_bonds_at_start = true;
        }
        if (triggered_tank_bonds_at_start && tank_bonding.action_state == ActionState::INACTIVE)
        {
            moves[0].Execute();
            triggered_tank_bonds_at_start = false;
            triggered_move_start = true;
            send_move = true;
        }
        if (triggered_move_start && move_idx == 1 && TankIsSoloLT())
        {
            pumping.action_state = ActionState::ACTIVE;
        }

        WarnDistanceLT();
    }
    // END Automated Actions at UW Entry

    tank_bonding.Update();
    player_bonding.Update();
    fuse_pull.Update();
    pumping.Update();
}

bool EmoWindow::ActivationConditions() const
{
    if (!GW::PartyMgr::GetIsPartyLoaded())
        return false;

    if (player.primary == GW::Constants::Profession::Elementalist &&
        player.secondary == GW::Constants::Profession::Monk)
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

            uint32_t player_number = (pak->agent_type ^ 0x20000000);

            if (!IsUw() || player_number != 514) // Turtle id
                return;

            found_turtle = true;
            turtle_id = pak->agent_id;
        });
}

RoutineState Pumping::RoutineSelfBonds() const
{
    const auto found_ether = player->HasEffect(GW::Constants::SkillID::Ether_Renewal);
    if (skillbar->ether.CanBeCasted(player->energy))
        return SafeUseSkill(skillbar->ether.idx, player->id);

    if (player->energy_perc > 0.5)
    {
        const auto has_balth = player->HasBuff(GW::Constants::SkillID::Balthazars_Spirit);
        const auto balth_avail = skillbar->balth.CanBeCasted(player->energy);
        if (!has_balth && balth_avail)
            return SafeUseSkill(skillbar->balth.idx, player->id);

        const auto has_prot = player->HasBuff(GW::Constants::SkillID::Protective_Bond);
        const auto bond_avail = skillbar->prot.CanBeCasted(player->energy);
        if (!has_prot && bond_avail)
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

RoutineState Pumping::RoutineCanthaGuards() const
{
    static auto last_gdw_idx = 0;

    if (!skillbar->prot.SkillFound() || !skillbar->fuse.SkillFound())
        return RoutineState::ACTIVE;

    if (!player->CanCast())
        return RoutineState::ACTIVE;

    const auto agents_array = GW::Agents::GetAgentArray();
    auto filtered_canthas = std::vector<GW::AgentLiving *>{};
    FilterAgents(*player,
                 agents_array,
                 filtered_canthas,
                 cantha_ids,
                 GW::Constants::Allegiance::Npc_Minipet,
                 GW::Constants::Range::Spellcast);

    if (filtered_canthas.size() == 0)
        return RoutineState::ACTIVE;

    for (const auto &cantha : filtered_canthas)
    {
        if (!cantha || cantha->GetIsDead() && cantha->hp == 0.0F)
            continue;

        if (cantha->hp < 0.90F)
        {
            const auto has_prot = AgentHasBuff(GW::Constants::SkillID::Protective_Bond, cantha->agent_id);
            if (!has_prot && skillbar->prot.CanBeCasted(player->energy))
                return SafeUseSkill(skillbar->prot.idx, cantha->agent_id);
        }

        if (cantha->hp < 0.70F && skillbar->fuse.CanBeCasted(player->energy))
            return SafeUseSkill(skillbar->fuse.idx, cantha->agent_id);

        if (skillbar->gdw.CanBeCasted(player->energy) && !cantha->GetIsWeaponSpelled())
            return SafeUseSkill(skillbar->gdw.idx, cantha->agent_id);
    }

    return RoutineState::ACTIVE;
}

RoutineState Pumping::RoutineLT() const
{
    if (!lt_agent || !player->target || player->target->agent_id != lt_agent->agent_id)
        return RoutineState::ACTIVE;

    const auto target_living = player->target->GetAsAgentLiving();
    if (!target_living || target_living->GetIsMoving() || player->living->GetIsMoving() ||
        target_living->primary != static_cast<uint8_t>(GW::Constants::Profession::Mesmer))
        return RoutineState::ACTIVE;

    const auto dist = GW::GetDistance(player->pos, player->target->pos);

    if (dist < FuseRange::FUSE_PULL_RANGE - 5.0F || dist > FuseRange::FUSE_PULL_RANGE + 5.0F)
        return RoutineState::ACTIVE;

    if (skillbar->fuse.CanBeCasted(player->energy) && target_living->hp < 0.80F)
        return SafeUseSkill(skillbar->fuse.idx, target_living->agent_id);

    if (skillbar->sb.CanBeCasted(player->energy))
        return SafeUseSkill(skillbar->sb.idx, target_living->agent_id);

    return RoutineState::ACTIVE;
}

RoutineState Pumping::RoutineTurtle() const
{
    if (!found_turtle || !turtle_id)
        return RoutineState::ACTIVE;

    const auto turtle_agent = GW::Agents::GetAgentByID(turtle_id);
    if (!turtle_agent)
        return RoutineState::ACTIVE;

    const auto dist = GW::GetDistance(player->pos, turtle_agent->pos);

    if (dist > GW::Constants::Range::Spellcast)
        return RoutineState::ACTIVE;

    const auto has_prot = AgentHasBuff(GW::Constants::SkillID::Protective_Bond, turtle_agent->agent_id);
    if (!has_prot)
        return SafeUseSkill(skillbar->prot.idx, turtle_agent->agent_id);

    const auto has_life = AgentHasBuff(GW::Constants::SkillID::Life_Bond, turtle_agent->agent_id);
    if (!has_life)
        return SafeUseSkill(skillbar->life.idx, turtle_agent->agent_id);

    const auto turtle_living = turtle_agent->GetAsAgentLiving();
    if (turtle_living && turtle_living->hp < 0.95F)
        return SafeUseSkill(skillbar->fuse.idx, turtle_agent->agent_id);

    return RoutineState::ACTIVE;
}

RoutineState Pumping::RoutinePI(const uint32_t dhuum_id) const
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

RoutineState Pumping::RoutineWisdom() const
{
    const auto &skill = skillbar->wisdom;

    if (!skill.SkillFound())
        return RoutineState::ACTIVE;

    const auto wisdom_avail = skill.CanBeCasted(player->energy);
    if (wisdom_avail)
        return SafeUseSkill(skill.idx);

    return RoutineState::ACTIVE;
}

RoutineState Pumping::RoutineGDW() const
{
    static auto last_idx = uint32_t{0};

    if (last_idx >= GW::PartyMgr::GetPartySize())
        last_idx = 0;

    const auto &skill = skillbar->gdw;

    if (!skill.SkillFound() || !player->CanCast())
        return RoutineState::ACTIVE;

    std::vector<PlayerMapping> party_members;
    const auto success = GetPartyMembers(party_members);

    if (!success)
    {
        last_idx++;
        return RoutineState::ACTIVE;
    }

    const auto id = party_members[last_idx].id;

    if (id == player->id)
    {
        last_idx++;
        return RoutineState::ACTIVE;
    }

    const auto agent = GW::Agents::GetAgentByID(id);
    if (!agent)
    {
        last_idx++;
        return RoutineState::ACTIVE;
    }

    const auto living = agent->GetAsAgentLiving();
    if (!living || living->GetIsMoving())
    {
        last_idx++;
        return RoutineState::ACTIVE;
    }

    const auto dist = GW::GetDistance(player->pos, agent->pos);
    if (dist > GW::Constants::Range::Spellcast)
    {
        last_idx++;
        return RoutineState::ACTIVE;
    }

    if (living->GetIsWeaponSpelled())
    {
        last_idx++;
        return RoutineState::ACTIVE;
    }

    last_idx++;
    return SafeUseSkill(skill.idx, id);
}

RoutineState Pumping::RoutineKeepPlayerAlive() const
{
    std::vector<PlayerMapping> party_members;
    const auto success = GetPartyMembers(party_members);

    for (const auto &[id, idx] : party_members)
    {
        if (!id || id == player->id)
            continue;

        const auto agent = GW::Agents::GetAgentByID(id);
        if (!agent)
            continue;

        const auto living = agent->GetAsAgentLiving();
        if (!living)
            continue;

        const auto has_prot = AgentHasBuff(GW::Constants::SkillID::Protective_Bond, living->agent_id);
        const auto prot_avail = skillbar->fuse.CanBeCasted(player->energy);
        if (!has_prot && prot_avail && living->hp < 0.70F &&
            living->primary != static_cast<uint8_t>(GW::Constants::Profession::Ranger))
        {
            return SafeUseSkill(skillbar->prot.idx, living->agent_id);
        }

        const auto fuse_avail = skillbar->fuse.CanBeCasted(player->energy);
        if (fuse_avail && living->hp < 0.40F)
        {
            return SafeUseSkill(skillbar->fuse.idx, living->agent_id);
        }
    }

    return RoutineState::FINISHED;
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

    if (IsInVale(player))
    {
        const auto cantha_state = RoutineCanthaGuards();
        if (cantha_state == RoutineState::FINISHED)
            return RoutineState::FINISHED;
    }

    static auto swapped_armor_in_dhuum_room = false;
    const auto is_in_dhuum_room = IsInDhuumRoom(player);
    if (!is_in_dhuum_room)
    {
        swapped_armor_in_dhuum_room = false;
        return RoutineState::FINISHED;
    }

    const auto swap_to_low_armor = !swapped_armor_in_dhuum_room && is_in_dhuum_room;
    if (swap_to_low_armor)
    {
        swapped_armor_in_dhuum_room = true;
        if (bag_idx && start_slot_idx)
            ChangeFullArmor(*bag_idx, *start_slot_idx);
    }

    const auto turtle_state = RoutineTurtle();
    if (turtle_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    auto dhuum_id = uint32_t{0};
    auto dhuum_hp = float{1.0F};
    static auto dhuum_fight_started = false;
    const auto is_in_dhuum_fight = IsInDhuumFight(&dhuum_id, &dhuum_hp);

    if (!is_in_dhuum_fight || !dhuum_id || dhuum_hp == 1.0F)
        return RoutineState::FINISHED;

    if (is_in_dhuum_fight && !dhuum_fight_started)
        dhuum_fight_started = true;

    const auto wisdom_state = RoutineWisdom();
    if (wisdom_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    const auto pi_state = RoutinePI(dhuum_id);
    if (pi_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    if (!dhuum_hp || dhuum_hp > 0.99F)
        return RoutineState::FINISHED;

    const auto gdw_state = RoutineGDW();
    if (gdw_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    const auto keep_player_alive_state = RoutineKeepPlayerAlive();
    if (keep_player_alive_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    return RoutineState::FINISHED;
}

void Pumping::Update()
{
    static auto paused = false;
    static auto paused_by_reaper = false;

    if (IsUw())
    {
        const auto lt_id = GetTankId();
        lt_agent = GW::Agents::GetAgentByID(lt_id);
    }

    if (player->living->GetIsMoving() && action_state == ActionState::ACTIVE)
    {
        action_state = ActionState::ON_HOLD;
        paused = true;
    }
    else if (player->target && action_state == ActionState::ACTIVE)
    {
        const auto dist = GW::GetDistance(player->pos, player->target->pos);

        if (TargetIsReaper(*player) && (dist < GW::Constants::Range::Earshot / 2.0F))
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
        if (action_state == ActionState::ON_HOLD)
            action_state = ActionState::ACTIVE;
    }
    else if (paused && paused_by_reaper && !TargetIsReaper(*player))
    {
        paused = false;
        paused_by_reaper = false;
        if (action_state == ActionState::ON_HOLD)
            action_state = ActionState::ACTIVE;
    }

    if (GW::PartyMgr::GetIsPartyDefeated())
        action_state = ActionState::INACTIVE;
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

    const auto has_balth = AgentHasBuff(GW::Constants::SkillID::Balthazars_Spirit, target_id);
    if (!has_balth && skillbar->balth.CanBeCasted(player->energy))
    {
        (void)SafeUseSkill(skillbar->balth.idx, target_id);
        return RoutineState::ACTIVE;
    }

    const auto has_prot = AgentHasBuff(GW::Constants::SkillID::Protective_Bond, target_id);
    if (!has_prot && skillbar->prot.CanBeCasted(player->energy))
    {
        (void)SafeUseSkill(skillbar->prot.idx, target_id);
        return RoutineState::ACTIVE;
    }

    const auto has_life = AgentHasBuff(GW::Constants::SkillID::Life_Bond, target_id);
    if (!has_life && skillbar->life.CanBeCasted(player->energy))
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

    const auto has_balth = AgentHasBuff(GW::Constants::SkillID::Balthazars_Spirit, target_id);
    if (!has_balth && skillbar->balth.CanBeCasted(player->energy))
    {
        (void)SafeUseSkill(skillbar->balth.idx, target_id);
        return RoutineState::ACTIVE;
    }

    const auto has_prot = AgentHasBuff(GW::Constants::SkillID::Protective_Bond, target_id);
    if (!has_prot && skillbar->prot.CanBeCasted(player->energy))
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

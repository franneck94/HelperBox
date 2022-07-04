#include <cstdint>
#include <set>
#include <tuple>
#include <vector>

#include <GWCA/GameContainers/Array.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Party.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/EffectMgr.h>
#include <GWCA/Managers/ItemMgr.h>
#include <GWCA/Managers/PartyMgr.h>

#include <Helper.h>
#include <PlayerData.h>
#include <SkillData.h>
#include <Types.h>

#include "HelperPlayer.h"

bool CanMove()
{
    return !IsLoading() && !GW::Map::GetIsObserving();
}

bool IsAliveAlly(const GW::Agent *target)
{
    if (!target)
        return false;

    if (!target->GetIsLivingType())
        return false;

    const auto target_living = target->GetAsAgentLiving();
    if (!target_living)
        return false;

    if (target_living->allegiance != GW::Constants::Allegiance::Ally_NonAttackable || target_living->GetIsDead())
        return false;

    return true;
}

const GW::EffectArray *GetEffects(const uint32_t agent_id)
{
    const auto agent_effects = GW::Effects::GetPartyEffectsArray();

    if (!agent_effects || !agent_effects->valid())
        return nullptr;

    for (const auto &agent_effect : *agent_effects)
    {
        if (agent_effect.agent_id == agent_id && agent_effect.effects.valid())
            return &agent_effect.effects;
    }

    return nullptr;
}

bool TargetNearest(const TargetType type, const float max_distance)
{
    const auto agents = GW::Agents::GetAgentArray();
    if (!agents || !agents->valid())
        return false;

    const auto me = GW::Agents::GetPlayerAsAgentLiving();
    if (!me)
        return false;

    auto distance = max_distance;
    auto closest = size_t{0};

    for (const auto agent : *agents)
    {
        if (!agent || agent == me)
            continue;

        switch (type)
        {
        case TargetType::Gadget:
        {
            const auto gadget = agent->GetAsAgentGadget();
            if (!gadget)
                continue;
            break;
        }
        case TargetType::Item:
        {
            const auto item_agent = agent->GetAsAgentItem();
            if (!item_agent)
                continue;

            const auto item = GW::Items::GetItemById(item_agent->item_id);
            if (!item)
                continue;
            break;
        }
        case TargetType::Npc:
        {
            const auto living_agent = agent->GetAsAgentLiving();
            if (!living_agent || !living_agent->IsNPC() || !living_agent->GetIsAlive())
                continue;
            break;
        }
        case TargetType::PlayerData:
        {
            const auto living_agent = agent->GetAsAgentLiving();
            if (!living_agent || !living_agent->IsPlayer())
                continue;
            break;
        }
        case TargetType::Living_Ally:
        case TargetType::Living_Enemy:
        case TargetType::Living_Npc:
        {
            const auto living_agent = agent->GetAsAgentLiving();
            if (!living_agent || !living_agent->GetIsAlive())
                continue;

            if (type == TargetType::Living_Ally &&
                living_agent->allegiance == GW::Constants::Allegiance::Ally_NonAttackable)
                break;
            else if (type == TargetType::Living_Enemy && living_agent->allegiance == GW::Constants::Allegiance::Enemy)
                break;
            else if (type == TargetType::Living_Npc &&
                     living_agent->allegiance == GW::Constants::Allegiance::Npc_Minipet)
                break;

            continue;
        }
        default:
        {
            continue;
        }
        }

        const auto newDistance = GW::GetDistance(me->pos, agent->pos);
        if (newDistance < distance)
        {
            closest = agent->agent_id;
            distance = newDistance;
        }
    }

    if (closest)
    {
        GW::Agents::ChangeTarget(closest);
        return true;
    }

    return false;
}

bool DetectPlayerIsDead()
{
    const auto me = GW::Agents::GetPlayer();
    if (!me)
        return false;

    const auto living_me = me->GetAsAgentLiving();
    if (!living_me)
        return false;

    return living_me->GetIsDead();
}

std::tuple<uint32_t, uint32_t, float> GetEnergy()
{
    const auto me = GW::Agents::GetPlayer();
    if (!me)
        return std::make_tuple(0, 0, 0.0F);

    const auto living_me = me->GetAsAgentLiving();
    if (!living_me)
        return std::make_tuple(0, 0, 0.0F);

    const auto max_energy = living_me->max_energy;
    const auto energy_perc = living_me->energy;
    const auto energy = static_cast<float>(max_energy) * energy_perc;

    return std::make_tuple(static_cast<uint32_t>(energy), max_energy, energy_perc);
}

std::tuple<uint32_t, uint32_t, float> GetHp()
{
    const auto me = GW::Agents::GetPlayer();
    if (!me)
        return std::make_tuple(0, 0, 0.0F);

    const auto living_me = me->GetAsAgentLiving();
    if (!living_me)
        return std::make_tuple(0, 0, 0.0F);

    const auto max_hp = living_me->max_hp;
    const auto hp_perc = living_me->hp;
    const auto hp = static_cast<float>(max_hp) * hp_perc;

    return std::make_tuple(static_cast<uint32_t>(hp), max_hp, hp_perc);
}

bool AgentHasBuff(const GW::Constants::SkillID buff_skill_id, const uint32_t target_agent_id)
{
    const auto effects = GW::Effects::GetPartyEffectsArray();
    if (!effects || !effects->valid())
        return false;

    const auto &buffs = (*effects)[0].buffs;
    if (!buffs.valid())
        return false;

    for (size_t i = 0; i < buffs.size(); ++i)
    {
        const auto agent_id = buffs[i].target_agent_id;
        const auto skill_id = buffs[i].skill_id;

        if (agent_id == target_agent_id)
        {
            if (skill_id == static_cast<uint32_t>(buff_skill_id))
                return true;
        }
    }

    return false;
}

void TargetAndAttackEnemyInAggro(const PlayerData &player_data, const float range)
{
    if (!player_data.target || !player_data.target->agent_id || !player_data.target->GetIsLivingType() ||
        player_data.target->GetAsAgentLiving()->allegiance != GW::Constants::Allegiance::Enemy)
        TargetNearest(TargetType::Living_Enemy, range);

    if (player_data.target && player_data.target->agent_id)
    {
        const auto dist = GW::GetDistance(player_data.pos, player_data.target->pos);
        if (dist < range)
            AttackAgent(player_data.target);
    }
}

bool CastBondIfNotAvailable(const SkillData &skill_data, const uint32_t target_id, const PlayerData *const player_data)
{
    const auto has_bond = AgentHasBuff(static_cast<GW::Constants::SkillID>(skill_data.id), target_id);
    const auto bond_avail = skill_data.CanBeCasted(player_data->energy);

    if (!has_bond && bond_avail)
    {
        GW::GameThread::Enqueue([&]() { GW::SkillbarMgr::UseSkill(skill_data.idx, target_id); });
        return true;
    }
    return false;
}

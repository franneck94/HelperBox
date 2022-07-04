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

bool TargetNearest(const GW::GamePos &player_pos,
                   const std::vector<GW::AgentLiving *> &livings,
                   const float max_distance)
{
    auto distance = max_distance;
    auto closest = size_t{0};

    for (const auto living : livings)
    {
        if (!living)
            continue;

        const auto newDistance = GW::GetDistance(player_pos, living->pos);
        if (newDistance < distance)
        {
            closest = living->agent_id;
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

void TargetAndAttackEnemyInAggro(const PlayerData &player_data,
                                 const std::vector<GW::AgentLiving *> &enemies,
                                 const float range)
{
    if (!player_data.target || !player_data.target->agent_id || !player_data.target->GetIsLivingType() ||
        player_data.target->GetAsAgentLiving()->allegiance != GW::Constants::Allegiance::Enemy)
        TargetNearest(player_data.pos, enemies, range);

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

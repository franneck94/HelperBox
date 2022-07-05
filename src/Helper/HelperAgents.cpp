#include <algorithm>
#include <cstdint>
#include <iterator>
#include <set>
#include <tuple>
#include <vector>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/GameContainers/Array.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Party.h>
#include <GWCA/GameEntities/Player.h>
#include <GWCA/GameEntities/Skill.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/ItemMgr.h>
#include <GWCA/Managers/PartyMgr.h>

#include <Helper.h>
#include <HelperMaps.h>
#include <PlayerData.h>
#include <SkillData.h>
#include <Types.h>

#include "HelperAgents.h"

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

std::pair<GW::Agent *, float> GetClosestEnemy(const PlayerData *player_data)
{
    const auto agents = GW::Agents::GetAgentArray();
    if (!agents || !agents->valid())
        return std::make_pair(nullptr, 0.0F);

    GW::Agent *closest = nullptr;
    auto closest_dist = FLT_MAX;

    for (const auto agent : *agents)
    {
        if (!agent)
            continue;

        const auto living = agent->GetAsAgentLiving();
        if (!living)
            continue;

        if (living->allegiance != GW::Constants::Allegiance::Enemy)
            continue;

        const auto dist = GW::GetDistance(player_data->pos, living->pos);
        if (dist < closest_dist)
            closest = agent;
    }

    return std::make_pair(closest, closest_dist);
}

uint32_t GetClosesTypeID(const PlayerData &player_data, const uint32_t id, const GW::Constants::Allegiance type)
{
    std::vector<GW::AgentLiving *> agents_vec;
    FilterAgents(player_data, agents_vec, std::array<uint32_t, 1>{id}, type, GW::Constants::Range::Compass);

    if (agents_vec.size() == 0)
        return 0;

    return agents_vec[0]->agent_id;
}

uint32_t GetClosestEnemyTypeID(const PlayerData &player_data, const uint32_t id)
{
    return GetClosesTypeID(player_data, id, GW::Constants::Allegiance::Enemy);
}

uint32_t GetClosestAllyTypeID(const PlayerData &player_data, const uint32_t id)
{
    return GetClosesTypeID(player_data, id, GW::Constants::Allegiance::Ally_NonAttackable);
}

uint32_t GetClosestNpcTypeID(const PlayerData &player_data, const uint32_t id)
{
    return GetClosesTypeID(player_data, id, GW::Constants::Allegiance::Npc_Minipet);
}

uint32_t TargetClosestEnemyById(PlayerData &player_data, const uint32_t id)
{
    const auto target_id = GetClosestEnemyTypeID(player_data, id);
    if (!target_id)
        return 0;

    player_data.ChangeTarget(target_id);

    return target_id;
}

uint32_t TargetClosestAllyById(PlayerData &player_data, const uint32_t id)
{
    const auto target_id = GetClosestAllyTypeID(player_data, id);
    if (!target_id)
        return 0;

    player_data.ChangeTarget(target_id);

    return target_id;
}

uint32_t TargetClosestNpcById(PlayerData &player_data, const uint32_t id)
{
    const auto target_id = GetClosestNpcTypeID(player_data, id);
    if (!target_id)
        return 0;

    player_data.ChangeTarget(target_id);

    return target_id;
}

void SortByDistance(const PlayerData &player_data, std::vector<GW::AgentLiving *> &filtered_livings)
{
    const auto player_pos = player_data.pos;

    std::sort(filtered_livings.begin(), filtered_livings.end(), [&player_pos](const auto a1, const auto a2) {
        const auto sqrd1 = GW::GetSquareDistance(player_pos, a1->pos);
        const auto sqrd2 = GW::GetSquareDistance(player_pos, a2->pos);
        return sqrd1 < sqrd2;
    });
}

std::vector<GW::AgentLiving *> FilterAgentsByRange(const std::vector<GW::AgentLiving *> &livings,
                                                   const PlayerData &player_data,
                                                   const float dist_threshold)
{
    auto filtered_livings = std::vector<GW::AgentLiving *>{};

    for (const auto living : livings)
    {
        const auto dist = GW::GetDistance(player_data.pos, living->pos);
        if (dist <= dist_threshold)
            filtered_livings.push_back(living);
    }

    return filtered_livings;
}

uint32_t GetPartyIdxByID(const uint32_t id)
{
    std::vector<PlayerMapping> party_members;
    const auto success = GetPartyMembers(party_members);

    if (!success)
        return 0U;

    const auto it = std::find_if(party_members.begin(), party_members.end(), [&id](const PlayerMapping &member) {
        return member.id == static_cast<uint32_t>(id);
    });
    if (it == party_members.end())
        return 0U;

    const auto idx = static_cast<uint32_t>(std::distance(party_members.begin(), it));
    if (idx >= GW::PartyMgr::GetPartySize())
        return 0U;

    return idx;
}

void SplitFilteredAgents(const std::vector<GW::AgentLiving *> &filtered_livings,
                         std::vector<GW::AgentLiving *> &splitted_agents,
                         const uint32_t id)
{
    for (auto agent : filtered_livings)
    {
        if (agent->player_number == id)
            splitted_agents.push_back(agent);
    }
}

bool GetPartyMembers(std::vector<PlayerMapping> &party_members)
{
    if (!GW::PartyMgr::GetIsPartyLoaded())
        return false;
    if (!GW::Map::GetIsMapLoaded())
        return false;

    const auto info = GW::PartyMgr::GetPartyInfo();
    if (!info)
        return false;

    const auto players = GW::Agents::GetPlayerArray();
    if (!players || !players->valid())
        return false;

    party_members.clear();

    auto idx = uint32_t{0};
    for (const auto &player_data : info->players)
    {
        const auto id = (*players)[player_data.login_number].agent_id;
        party_members.push_back({id, idx});
        ++idx;

        for (const auto &hero : info->heroes)
        {
            if (hero.owner_player_id == player_data.login_number)
            {
                party_members.push_back({hero.agent_id, idx});
                ++idx;
            }
        }
    }
    for (const auto &hench : info->henchmen)
    {
        party_members.push_back({hench.agent_id, idx});
        ++idx;
    }

    return true;
}

std::set<uint32_t> FilterAgentIDS(const std::vector<GW::AgentLiving *> &filtered_livings,
                                  const std::set<uint32_t> &filter_ids)
{
    auto set_ids = std::set<uint32_t>{};
    auto result_ids = std::set<uint32_t>{};
    for (const auto living : filtered_livings)
    {
        set_ids.insert(static_cast<uint32_t>(living->player_number));
    }
    std::set_difference(set_ids.begin(),
                        set_ids.end(),
                        filter_ids.begin(),
                        filter_ids.end(),
                        std::inserter(result_ids, result_ids.end()));

    return result_ids;
}

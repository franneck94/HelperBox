#pragma once

#include <cstdint>
#include <set>
#include <tuple>
#include <vector>

#include <GWCA/GameEntities/Agent.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/EffectMgr.h>
#include <GWCA/Managers/ItemMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/PartyMgr.h>

#include <AgentData.h>
#include <PlayerData.h>
#include <SkillData.h>
#include <Types.h>

bool CanMove();

bool IsAliveAlly(const GW::Agent *target);

const GW::EffectArray *GetEffects(const uint32_t agent_id);

bool DetectPlayerIsDead();

std::tuple<uint32_t, uint32_t, float> GetEnergy();

std::tuple<uint32_t, uint32_t, float> GetHp();

bool AgentHasBuff(const GW::Constants::SkillID buff_skill_id, const uint32_t target_agent_id);

void TargetAndAttackEnemyInAggro(const PlayerData &player_data,
                                 const std::vector<GW::AgentLiving *> &enemies,
                                 const float range = GW::Constants::Range::Spellcast);

bool CastBondIfNotAvailable(const SkillData &skill_data, const uint32_t target_id, const PlayerData *const player_data);

bool TargetNearest(const GW::GamePos &player_pos,
                   const std::vector<GW::AgentLiving *> &livings,
                   const float max_distance = GW::Constants::SqrRange::Compass);

template <uint32_t N>
void FilterAgents(const PlayerData &player_data,
                  std::vector<GW::AgentLiving *> &filtered_livings,
                  const std::array<uint32_t, N> &ids,
                  const GW::Constants::Allegiance allegiance,
                  const float max_distance = 0.0F)
{
    const auto agents_array = GW::Agents::GetAgentArray();
    if (!agents_array || !agents_array->valid())
        return;

    for (const auto agent : *agents_array)
    {
        if (!agent)
            continue;

        const auto living = agent->GetAsAgentLiving();

        if (!living)
            continue;

        if (living->allegiance != allegiance)
            continue;

        for (const auto id : ids)
        {
            if (living->GetIsDead())
                continue;

            if (living->player_number == id)
            {
                if (max_distance == 0.0F)
                {
                    filtered_livings.push_back(living);
                }
                else
                {
                    const auto dist = GW::GetDistance(player_data.pos, agent->pos);

                    if (dist < max_distance)
                    {
                        filtered_livings.push_back(living);
                    }
                }
            }
        }
    }
}

void FilterByIdAndDistance(const GW::GamePos &player_pos,
                           const std::vector<GW::AgentLiving *> &livings,
                           std::vector<GW::AgentLiving *> &filtered_livings,
                           const uint32_t id,
                           const float max_distance = GW::Constants::Range::Compass);

template <uint32_t N>
void FilterByIdsAndDistances(const GW::GamePos &player_pos,
                             const std::vector<GW::AgentLiving *> &livings,
                             std::vector<GW::AgentLiving *> &filtered_livings,
                             const std::array<uint32_t, N> ids,
                             const float max_distance = GW::Constants::Range::Compass)
{
    for (auto living : livings)
    {
        for (const auto id : ids)
        {
            if (living->player_number == id && GW::GetDistance(player_pos, living->pos) < max_distance)
                filtered_livings.push_back(living);
        }
    }
}

void SortByDistance(const PlayerData &player_data, std::vector<GW::AgentLiving *> &filtered_livings);

std::pair<GW::Agent *, float> GetClosestEnemy(const PlayerData *player_data);

uint32_t GetClosestById(const PlayerData &player_data,
                        const std::vector<GW::AgentLiving *> &livings,
                        const uint32_t id);

uint32_t GetClosestEnemyById(const PlayerData &player_data,
                             const std::vector<GW::AgentLiving *> &enemies,
                             const uint32_t id);

uint32_t GetClosestAllyById(const PlayerData &player_data,
                            const std::vector<GW::AgentLiving *> &allies,
                            const uint32_t id);

uint32_t GetClosestNpcbyId(const PlayerData &player_data,
                           const std::vector<GW::AgentLiving *> &npcs,
                           const uint32_t id);

uint32_t TargetClosestEnemyById(PlayerData &player_data,
                                const std::vector<GW::AgentLiving *> &enemies,
                                const uint32_t id);

uint32_t TargetClosestAllyById(PlayerData &player_data,
                               const std::vector<GW::AgentLiving *> &allies,
                               const uint32_t id);

uint32_t TargetClosestNpcById(PlayerData &player_data, const std::vector<GW::AgentLiving *> &npcs, const uint32_t id);

std::vector<GW::AgentLiving *> FilterAgentsByRange(const std::vector<GW::AgentLiving *> &livings,
                                                   const PlayerData &player_data,
                                                   const float dist_threshold = GW::Constants::Range::Earshot);

bool GetPartyMembers(std::vector<PlayerMapping> &party_members);

std::set<uint32_t> FilterAgentIDS(const std::vector<GW::AgentLiving *> &filtered_livings,
                                  const std::set<uint32_t> &filter_ids);

uint32_t GetPartyIdxByID(const uint32_t id);

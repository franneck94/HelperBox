#pragma once

#include <array>
#include <cstdint>
#include <set>
#include <string_view>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Skill.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/MapMgr.h>

#include <MathUtils.h>
#include <PlayerData.h>
#include <Types.h>

bool IsLoading();

bool IsExplorable();

bool IsOutpost();

bool IsMapReady();

bool IsEndGameEntryOutpost();

bool IsFowEntryOutpost();

bool IsDoa();

bool IsDoaEntryOutpost();

bool HelperActivationConditions();

bool TargetNearest(const TargetType type, const float max_distance = GW::Constants::SqrRange::Compass);

bool EquipItemExecute(const uint32_t bag_idx, const uint32_t slot_idx);

bool ArmorSwap(const uint32_t bag_idx, const uint32_t start_slot_idx, const uint32_t armor_threshold);

bool LowArmor(const uint32_t bag_idx, const uint32_t start_slot_idx);

bool HighArmor(const uint32_t bag_idx, const uint32_t start_slot_idx);

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

void SplitFilteredAgents(const std::vector<GW::AgentLiving *> &filtered_livings,
                         std::vector<GW::AgentLiving *> &splitted_agents,
                         const uint32_t id);

void SortByDistance(const PlayerData &player_data, std::vector<GW::AgentLiving *> &filtered_livings);

uint32_t GetPartyIdxByID(const uint32_t id);

std::pair<GW::Agent *, float> GetClosestEnemy(const PlayerData *player_data);

uint32_t GetClosesTypeID(const PlayerData &player_data, const uint32_t id, const GW::Constants::Allegiance type);

uint32_t GetClosestEnemyTypeID(const PlayerData &player_data, const uint32_t id);

uint32_t GetClosestAllyTypeID(const PlayerData &player_data, const uint32_t id);

uint32_t GetClosestNpcTypeID(const PlayerData &player_data, const uint32_t id);

uint32_t TargetClosestEnemyById(PlayerData &player_data, const uint32_t id);

uint32_t TargetClosestAllyById(PlayerData &player_data, const uint32_t id);

uint32_t TargetClosestNpcById(PlayerData &player_data, const uint32_t id);

DWORD QuestAcceptDialog(DWORD quest);

DWORD QuestRewardDialog(DWORD quest);

bool GetPartyMembers(std::vector<PlayerMapping> &party_members);

std::vector<GW::AgentLiving *> FilterAgentsByRange(const std::vector<GW::AgentLiving *> &livings,
                                                   const PlayerData &player_data,
                                                   const float dist_threshold = GW::Constants::Range::Earshot);

std::set<uint32_t> FilterAgentIDS(const std::vector<GW::AgentLiving *> &filtered_livings,
                                  const std::set<uint32_t> &filter_ids);

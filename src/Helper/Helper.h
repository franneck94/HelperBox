#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Skill.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/MapMgr.h>

#include <MathUtils.h>
#include <Player.h>
#include <Types.h>

bool IsLoading();

bool IsExplorable();

bool IsOutpost();

bool IsMapReady();

bool IsFowEntryOutpost();

bool IsDoa();

bool IsDoaEntryOutpost();

bool TargetNearest(const TargetType type, const float max_distance = GW::Constants::SqrRange::Compass);

const GW::EffectArray *GetEffects(const uint32_t agent_id);

bool DetectPlayerIsDead();

std::tuple<uint32_t, uint32_t, float> GetEnergy();

std::tuple<uint32_t, uint32_t, float> GetHp();

bool AgentHasBuff(const GW::Constants::SkillID buff_skill_id, const uint32_t target_agent_id);

bool GetPartyMembers(std::vector<PlayerMapping> &party_members);

bool EquipItemExecute(const uint32_t bag_idx, const uint32_t slot_idx);

void ChangeFullArmor(const uint32_t bag_idx, const uint32_t start_slot_idx);

void ToLowArmor(const uint32_t bag_idx, const uint32_t start_slot_idx);

void ToHighArmor(const uint32_t bag_idx, const uint32_t start_slot_idx);

template <uint32_t N>
void FilterAgents(const Player &player,
                  const GW::AgentArray &agents,
                  std::vector<GW::AgentLiving *> &filtered_agents,
                  const std::array<uint32_t, N> &ids,
                  const GW::Constants::Allegiance allegiance,
                  const float max_distance = 0.0F)
{
    for (const auto &agent : agents)
    {
        if (!agent)
            continue;

        const auto living = agent->GetAsAgentLiving();

        if (!living)
            continue;

        if (living->allegiance != static_cast<uint8_t>(allegiance))
            continue;

        for (const auto id : ids)
        {
            if (living->GetIsDead())
                continue;

            if (living->player_number == id)
            {
                if (max_distance == 0.0F)
                {
                    filtered_agents.push_back(living);
                }
                else
                {
                    const auto dist = GW::GetDistance(player.pos, agent->pos);

                    if (dist < max_distance)
                    {
                        filtered_agents.push_back(living);
                    }
                }
            }
        }
    }
}

template <uint32_t N, uint32_t M>
void FilterAgentsAtPositionWithDistance(const GW::GamePos &pos,
                                        const GW::AgentArray &agents,
                                        std::vector<GW::AgentLiving *> &filtered_agents,
                                        const std::array<uint32_t, N> &ids,
                                        const std::array<uint32_t, M> &filter_ids,
                                        const GW::Constants::Allegiance allegiance,
                                        const float max_distance = 0.0F)
{
    for (const auto &agent : agents)
    {
        if (!agent)
            continue;

        const auto living = agent->GetAsAgentLiving();

        if (!living)
            continue;

        if (living->allegiance != static_cast<uint8_t>(allegiance))
            continue;

        if (ids.size() == 0)
        {
            if (living->GetIsDead())
                continue;

            auto skip = false;
            for (const auto filter_id : filter_ids)
            {
                if (living->player_number == filter_id)
                {
                    skip = true;
                    break;
                }
            }
            if (skip)
                continue;

            if (max_distance == 0.0F)
            {
                filtered_agents.push_back(living);
            }
            else
            {
                const auto dist = GW::GetDistance(pos, agent->pos);

                if (dist < max_distance)
                    filtered_agents.push_back(living);
            }
        }
        else
        {
            for (const auto id : ids)
            {
                if (living->GetIsDead())
                    continue;

                if (living->player_number == id)
                {
                    if (max_distance == 0.0F)
                    {
                        filtered_agents.push_back(living);
                    }
                    else
                    {
                        const auto dist = GW::GetDistance(pos, agent->pos);

                        if (dist < max_distance)
                            filtered_agents.push_back(living);
                    }
                }
            }
        }
    }
}

void SplitFilteredAgents(const std::vector<GW::AgentLiving *> &filtered_agents,
                         std::vector<GW::AgentLiving *> &splitted_agents,
                         const uint32_t id);

void SortByDistance(const Player &player, std::vector<GW::AgentLiving *> &filtered_agents);

bool CanMove();

uint32_t GetTankId();

uint32_t GetEmoId();

uint32_t GetDhuumBitchId();

bool IsAliveAlly(const GW::Agent *target);

uint32_t GetPartyIdxByID(const uint32_t id);

std::pair<GW::Agent *, float> GetClosestEnemy(const Player *player);

uint32_t GetClosesTypeID(const Player &player, const uint32_t id, const GW::Constants::Allegiance type);

uint32_t GetClosestEnemyTypeID(const Player &player, const uint32_t id);

uint32_t GetClosestAllyTypeID(const Player &player, const uint32_t id);

uint32_t GetClosestNpcTypeID(const Player &player, const uint32_t id);

void TargetClosestEnemyById(Player &player, const uint32_t id);

void TargetClosestAllyById(Player &player, const uint32_t id);

void TargetClosestNpcById(Player &player, const uint32_t id);

DWORD QuestAcceptDialog(DWORD quest);

DWORD QuestRewardDialog(DWORD quest);

void AttackAgent(const GW::Agent *agent);

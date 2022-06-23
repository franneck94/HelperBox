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
                  std::vector<GW::AgentLiving *> &filtered_livings,
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
                    filtered_livings.push_back(living);
                }
                else
                {
                    const auto dist = GW::GetDistance(player.pos, agent->pos);

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

void SortByDistance(const Player &player, std::vector<GW::AgentLiving *> &filtered_livings);

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

uint32_t TargetClosestEnemyById(Player &player, const uint32_t id);

uint32_t TargetClosestAllyById(Player &player, const uint32_t id);

uint32_t TargetClosestNpcById(Player &player, const uint32_t id);

DWORD QuestAcceptDialog(DWORD quest);

DWORD QuestRewardDialog(DWORD quest);

void AttackAgent(const GW::Agent *agent);

std::vector<GW::AgentLiving *> GetEnemiesInCompass();

std::vector<GW::AgentLiving *> GetEnemiesInGameRectangle(const GameRectangle &rectangle);

std::vector<GW::AgentLiving *> GetEnemiesInAggro(const Player &player);

std::set<uint32_t> FilterAgentIDS(const std::vector<GW::AgentLiving *> &filtered_livings,
                                  const std::set<uint32_t> &filter_ids);

void TargetAndAttackEnemyInAggro(const Player &player);

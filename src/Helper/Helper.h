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

void SplitFilteredAgents(const std::vector<GW::AgentLiving *> &filtered_agents,
                         std::vector<GW::AgentLiving *> &splitted_agents,
                         const uint32_t id);

void SortByDistance(const Player &player, std::vector<GW::AgentLiving *> &filtered_agents);

bool CanMove();

uint32_t GetTankId();

bool IsAliveAlly(const GW::Agent *target);

uint32_t GetPartyIdxByID(const uint32_t id);

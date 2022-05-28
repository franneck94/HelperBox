#pragma once

#include <cstdint>
#include <string_view>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Skill.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/MapMgr.h>

#include <Logger.h>

#include <Player.h>
#include <Skillbars.h>
#include <Types.h>
#include <Utils.h>

bool IsLoading();

bool IsExplorable();

bool IsOutpost();

bool IsMapReady();

bool IsUwEntryOutpost();

bool IsFowEntryOutpost();

bool TargetNearest(const TargetType type, const float max_distance = GW::Constants::SqrRange::Compass);

GW::EffectArray *GetEffects(const uint32_t agent_id);

bool PartyPlayerHasEffect(const GW::Constants::SkillID effect_skill_id, const uint32_t party_idx);

bool DetectNotMoving(const uint32_t threshold = 100U);

bool DetectPlayerIsDead();

std::tuple<uint32_t, uint32_t, float> GetEnergy();

std::tuple<uint32_t, uint32_t, float> GetHp();

bool AgentHasBuff(const GW::Constants::SkillID buff_skill_id, const uint32_t target_agent_id);

bool GetPartyMembers(std::vector<PlayerMapping> &party_members);

bool EquipItemExecute(const uint32_t bag_idx, const uint32_t slot_idx);

void ChangeFullArmor(const uint32_t bag_idx, const uint32_t start_slot_idx);

void FilterAgents(const Player &player,
                  const GW::AgentArray &agents,
                  std::vector<GW::AgentLiving *> &filtered_agents,
                  const int id,
                  const float max_distance = 0.0F);

void SortByDistance(const Player &player, std::vector<GW::AgentLiving *> &filtered_agents);

bool IsInDhuumRoom(const Player *player);

bool IsInDhuumFight(uint32_t *dhuum_id = nullptr);

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

void AttackAgent(const GW::Agent *agent);

void TargetAndAttackEnemyInAggro(const PlayerData &player_data, const float range = GW::Constants::Range::Spellcast);

bool CastBondIfNotAvailable(const SkillData &skill_data, const uint32_t target_id, const PlayerData *const player_data);

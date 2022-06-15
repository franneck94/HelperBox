#pragma once

#include <Skillbars.h>
#include <Types.h>

RoutineState SafeWalk(const GW::GamePos target_position, const bool reset = false);

RoutineState SafeUseSkill(const uint32_t skill_idx, const uint32_t target = 0, const uint32_t call_target = 0);

bool CastBondIfNotAvailable(const SkillData &skill_data, const uint32_t target_id, const Player *const player);

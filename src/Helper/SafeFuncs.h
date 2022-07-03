#pragma once

#include <Skillbars.h>
#include <Types.h>

RoutineState SafeWalk(const GW::GamePos target_position, const bool reset = false);

bool CastBondIfNotAvailable(const SkillData &skill_data, const uint32_t target_id, const PlayerData *const player_data);

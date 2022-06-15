#pragma once

#include <cstdint>

#include <MathUtils.h>
#include <Player.h>
#include <Types.h>

bool IsUwEntryOutpost();

bool IsUw();

bool IsInVale(const Player *const player);

bool IsAtFusePulls(const Player *const player);

bool IsInDhuumRoom(const Player *const player);

bool IsInDhuumFight(uint32_t *dhuum_id, float *dhuum_hp);

uint32_t GetClosestReaperID(const Player &player);

void TargetClosestReaper(Player &player);

bool TankIsFullteamLT();

bool TankIsSoloLT();

bool TargetIsReaper(Player &player);

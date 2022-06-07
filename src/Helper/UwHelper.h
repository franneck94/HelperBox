#pragma once

#include <cstdint>

#include <Player.h>
#include <Types.h>
#include <Utils.h>

bool IsUwEntryOutpost();

bool IsUw();

bool IsInVale(Player *player);

bool IsInDhuumRoom(const Player *const player);

bool IsInDhuumFight(uint32_t *dhuum_id, float *dhuum_hp);

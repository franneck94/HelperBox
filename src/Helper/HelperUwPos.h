#pragma once

#include <GWCA/Constants/Constants.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/Managers/MapMgr.h>

bool IsUwEntryOutpost();

bool IsUw();

bool IsAtSpawn(const GW::GamePos &player_pos, const float range = 500.0F);

bool IsAtChamberSpike(const GW::GamePos &player_pos);

bool IsAtChamberMonuSpike(const GW::GamePos &player_pos);

bool IsAtChamberSkele(const GW::GamePos &player_pos);

bool IsInBasement(const GW::GamePos &player_pos);

bool IsAtBasementSkele(const GW::GamePos &player_pos);

bool InBackPatrolArea(const GW::GamePos &player_pos);

bool IsRightAtChamberSkele(const GW::GamePos &player_pos);

bool IsAtFusePulls(const GW::GamePos &player_pos);

bool IsInVale(const GW::GamePos &player_pos);

bool IsAtValeStart(const GW::GamePos &player_pos);

bool IsAtValeHouse(const GW::GamePos &player_pos);

bool IsRightAtValeHouse(const GW::GamePos &player_pos);

bool IsAtSpirits1(const GW::GamePos &player_pos);

bool IsAtSpirits2(const GW::GamePos &player_pos);

bool IsAtValeSpirits(const GW::GamePos &player_pos);

bool IsGoingToDhuum(const GW::GamePos &player_pos);

bool IsInDhuumRoom(const GW::GamePos &player_pos, const float range = GW::Constants::Range::Spellcast);

bool IsAtFilterSkelePos(const GW::GamePos &player_pos, const GW::GamePos &next_pos);

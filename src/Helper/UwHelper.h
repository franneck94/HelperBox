#pragma once

#include <cstdint>

#include <Actions.h>
#include <MathUtils.h>
#include <Player.h>
#include <Types.h>

bool IsUwEntryOutpost();

bool IsUw();

bool IsEmo(const Player &player);

bool IsDhuumBitch(const Player &player);

bool IsSpiker(const Player &player);

bool IsLT(const Player &player);

bool IsRangerTerra(const Player &player);

bool IsMesmerTerra(const Player &player);

bool IsAtValeHouse(const Player *const player);

bool IsSomewhereInVale(const Player *const player);

bool IsAtSpirits1(const Player *const player);

bool IsAtSpirits2(const Player *const player);

bool IsAtChamberSkele(const Player *const player);

bool IsInVale(const Player *const player);

bool IsAtFusePulls(const Player *const player);

bool IsInDhuumRoom(const Player *const player);

bool IsInDhuumFight(uint32_t *dhuum_id, float *dhuum_hp);

bool TankIsFullteamLT();

bool TankIsSoloLT();

bool TargetIsReaper(Player &player);

void TargetClosestReaper(Player &player);

void TalkClosestReaper(Player &player);

void TargetClosestKeeper(Player &player);

void AcceptChamber();

void TakeRestore();

void TakeEscort();

void TakeUWG();

void TakePits();

void TakePlanes();

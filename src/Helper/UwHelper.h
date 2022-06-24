#pragma once

#include <array>
#include <cstdint>

#include <Actions.h>
#include <MathUtils.h>
#include <Move.h>
#include <Player.h>
#include <Types.h>

#include <Logger.h>

bool IsUwEntryOutpost();

bool IsUw();

bool IsEmo(const Player &player);

bool IsDhuumBitch(const Player &player);

bool IsSpiker(const Player &player);

bool IsLT(const Player &player);

bool IsRangerTerra(const Player &player);

bool IsMesmerTerra(const Player &player);

bool IsAtChamberSkele(const Player &player);

bool IsRightAtChamberSkele(const Player &player);

bool IsAtFusePulls(const Player &player);

bool IsAtValeStart(const Player &player);

bool IsAtValeHouse(const Player &player);

bool IsRightAtValeHouse(const Player &player);

bool IsAtSpirits1(const Player &player);

bool IsAtSpirits2(const Player &player);

bool IsAtValeSpirits(const Player &player);

bool IsInDhuumRoom(const Player &player);

bool IsInDhuumFight(uint32_t *dhuum_id, float *dhuum_hp);

bool TankIsFullteamLT();

bool TankIsSoloLT();

bool TargetIsReaper(Player &player);

void TargetReaper(Player &player);

void TalkReaper(Player &player);

void TargetClosestKeeper(Player &player);

void AcceptChamber();

void TakeRestore();

void TakeEscort();

void TakeUWG();

void TakePits();

void TakePlanes();

template <uint32_t N>
void UpdateUwInfo(const Player &player, const std::array<Move, N> moves, uint32_t &move_idx)
{
    static auto called_first = true;
    static auto last_pos = player.pos;
    const auto at_spawn =
        GW::GetDistance(player.pos, GW::GamePos{1248.00F, 6965.51F, 0}) < GW::Constants::Range::Earshot;

    const auto curr_pos = player.pos;
    const auto dist = GW::GetDistance(last_pos, curr_pos);
    if (dist > 5'000.0F || (called_first && !at_spawn))
    {
        Log::Info("Ported!");
        move_idx = GetClostestMove(player, moves);
        called_first = false;
    }
    last_pos = curr_pos;
}

bool CheckKeeper(const std::vector<GW::AgentLiving *> &keeper_livings, const GW::GamePos &keeper_pos);

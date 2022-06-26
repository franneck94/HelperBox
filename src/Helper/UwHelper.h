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

bool UwHelperActivationConditions();

bool IsEmo(const Player &player);

bool IsDhuumBitch(const Player &player);

bool IsSpiker(const Player &player);

bool IsLT(const Player &player);

bool IsRangerTerra(const Player &player);

bool IsMesmerTerra(const Player &player);

bool IsAtSpawn(const Player &player);

bool IsAtChamberSkele(const Player &player);

bool IsAtBasementSkele(const Player &player);

bool IsRightAtChamberSkele(const Player &player);

bool IsAtFusePulls(const Player &player);

bool IsAtValeStart(const Player &player);

bool IsAtValeHouse(const Player &player);

bool IsRightAtValeHouse(const Player &player);

bool IsAtSpirits1(const Player &player);

bool IsAtSpirits2(const Player &player);

bool IsAtValeSpirits(const Player &player);

bool IsGoingToDhuum(const Player &player);

bool IsInDhuumRoom(const Player &player);

bool IsInDhuumFight(uint32_t *dhuum_id, float *dhuum_hp);

bool TankIsFullteamLT();

bool TankIsSoloLT();

bool TargetIsReaper(Player &player);

bool TargetReaper(Player &player);

bool TalkReaper(Player &player);

bool TargetClosestKeeper(Player &player);

bool AcceptChamber();

bool TakeRestore();

bool TakeEscort();

bool TakeUWG();

bool TakePits();

bool TakePlanes();

template <uint32_t N>
void UpdateUwInfo(const Player &player, const std::array<Move, N> moves, uint32_t &move_idx, const bool first_call)
{
    static auto last_pos = player.pos;

    if (move_idx >= moves.size() - 1)
        return;

    const auto curr_pos = player.pos;
    const auto port_detected = GW::GetDistance(last_pos, curr_pos) > GW::Constants::Range::Compass;
    const auto is_spawn = GW::GetDistance(GW::GamePos{1248.00F, 6965.51F, 0}, curr_pos) < GW::Constants::Range::Compass;

    const auto curr_move = moves[move_idx].pos;
    const auto curr_move_oob = GW::GetDistance(curr_move, curr_pos) > GW::Constants::Range::Compass;

    const auto next_move = moves[move_idx + 1].pos;
    const auto next_move_oob = GW::GetDistance(next_move, curr_pos) > GW::Constants::Range::Compass;

    if ((port_detected && next_move_oob) || (first_call && curr_move_oob && !is_spawn))
    {
        Log::Info("Ported!");
        move_idx = GetClostestMove(player, moves);
    }
    else if (port_detected && !next_move_oob)
    {
        ++move_idx;
    }
    last_pos = curr_pos;
}

bool FoundKeeperAtPos(const std::vector<GW::AgentLiving *> &keeper_livings, const GW::GamePos &keeper_pos);

bool DhuumIsCastingJudgement(const uint32_t dhuum_id);

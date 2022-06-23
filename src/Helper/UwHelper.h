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

template <uint32_t N>
void UpdatedUwMoves_Main(const Player &player, std::array<Move, N> &moves, uint32_t &move_idx, bool &move_ongoing)
{
    if (!move_ongoing)
        return;

    if (move_idx >= moves.size() - 1U)
        return;

    const auto finished = Move::UpdateMove(player, move_ongoing, moves[move_idx], moves[move_idx + 1U]);

    const auto is_moving = player.living->GetIsMoving();
    const auto reached_pos = GamePosCompare(player.pos, moves[move_idx].pos, 0.001F);

    if (!reached_pos && is_moving)
        return;

    const auto state = moves[move_idx].move_state;
    const auto is_proceeding_action = (state != MoveState::NONE);

    if (is_proceeding_action && !reached_pos && !is_moving && finished)
    {
        moves[move_idx].Execute();
        return;
    }

    if (finished)
    {
        move_ongoing = false;
        ++move_idx;
        if (is_proceeding_action)
        {
            move_ongoing = true;
            moves[move_idx].Execute();
        }
    }
}

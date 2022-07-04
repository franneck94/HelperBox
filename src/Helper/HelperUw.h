#pragma once
#include <array>
#include <cstdint>

#include <Actions.h>
#include <MathUtils.h>
#include <Move.h>
#include <PlayerData.h>
#include <Types.h>

#include <Logger.h>

bool UwHelperActivationConditions();

uint32_t GetTankId();

uint32_t GetEmoId();

uint32_t GetDhuumBitchId();

bool IsEmo(const PlayerData &player_data);

bool IsDhuumBitch(const PlayerData &player_data);

bool IsSpiker(const PlayerData &player_data);

bool IsLT(const PlayerData &player_data);

bool IsRangerTerra(const PlayerData &player_data);

bool IsMesmerTerra(const PlayerData &player_data);

bool IsInDhuumFight(uint32_t *dhuum_id, float *dhuum_hp, uint32_t *dhuum_max_hp = nullptr);

bool TankIsFullteamLT();

bool TankIsSoloLT();

bool TargetIsReaper(PlayerData &player_data);

bool TargetReaper(PlayerData &player_data);

bool TalkReaper(PlayerData &player_data);

bool TargetClosestKeeper(PlayerData &player_data);

bool AcceptChamber();

bool TakeRestore();

bool TakeEscort();

bool TakeUWG();

bool TakePits();

bool TakePlanes();

template <uint32_t N>
void UpdateUwInfo(const PlayerData &player_data,
                  const std::array<Move, N> &moves,
                  uint32_t &move_idx,
                  const bool first_call)
{
    static auto last_pos = player_data.pos;

    if (move_idx >= moves.size() - 1)
        return;

    const auto curr_pos = player_data.pos;
    const auto port_detected = GW::GetDistance(last_pos, curr_pos) > GW::Constants::Range::Compass;
    const auto is_spawn = GW::GetDistance(GW::GamePos{1248.00F, 6965.51F, 0}, curr_pos) < GW::Constants::Range::Compass;

    const auto curr_move = moves[move_idx].pos;
    const auto curr_move_oob = GW::GetDistance(curr_move, curr_pos) > GW::Constants::Range::Compass;

    const auto next_move = moves[move_idx + 1].pos;
    const auto next_move_oob = GW::GetDistance(next_move, curr_pos) > GW::Constants::Range::Compass;

    if ((port_detected && next_move_oob) || (first_call && curr_move_oob && !is_spawn))
    {
        Log::Info("Ported!");
        move_idx = GetFirstCloseMove(player_data, moves);
    }
    else if (port_detected && !next_move_oob)
    {
        ++move_idx;
    }
    last_pos = curr_pos;
}

bool FoundKeeperAtPos(const std::vector<GW::AgentLiving *> &keeper_livings, const GW::GamePos &keeper_pos);

bool DhuumIsCastingJudgement(const uint32_t dhuum_id);

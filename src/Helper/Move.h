#pragma once

#include <functional>
#include <map>
#include <optional>
#include <string>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/GameContainers/GamePos.h>

#include <Logger.h>
#include <Timer.h>

#include <Player.h>
#include <Skillbars.h>
#include <Types.h>

enum class MoveState
{
    NO_WAIT_AND_STOP,
    NO_WAIT_AND_CONTINUE,
    WAIT_AND_STOP,
    WAIT_AND_CONTINUE,
    DISTANCE_AND_CONTINUE,
    CAST_SKILL_AND_CONTINUE,
    CALLBACK_AND_CONTINUE,
};

class Move
{
public:
    // Move and then wait
    Move(const float _x, const float _y, const std::string &_name, const MoveState _move_state)
        : x(_x), y(_y), pos({x, y, 0}), name(_name), move_state(_move_state){};

    // Move and then wait for distance
    Move(const float _x,
         const float _y,
         const std::string &_name,
         const MoveState _move_state,
         const float _dist_threshold)
        : x(_x), y(_y), pos({x, y, 0}), name(_name), move_state(_move_state), dist_threshold(_dist_threshold){};

    // Move, trigger cb, and then wait
    Move(const float _x,
         const float _y,
         const std::string &_name,
         const MoveState _move_state,
         std::function<bool()> _trigger_cb)
        : x(_x), y(_y), pos({x, y, 0}), name(_name), trigger_cb(_trigger_cb), move_state(_move_state){};

    // Move, and cast skill at goal
    Move(const float _x,
         const float _y,
         const std::string &_name,
         const MoveState _move_state,
         const SkillData *_skill_cb)
        : x(_x), y(_y), pos({x, y, 0}), name(_name), move_state(_move_state), skill_cb(_skill_cb){};

    // Move, trigger cb, and cast skill at goal
    Move(const float _x,
         const float _y,
         const std::string &_name,
         const MoveState _move_state,
         std::function<bool()> _trigger_cb,
         const SkillData *_skill_cb)
        : x(_x), y(_y), pos({x, y, 0}), name(_name), trigger_cb(_trigger_cb), move_state(_move_state),
          skill_cb(_skill_cb){};

    const char *Name() const
    {
        return name.data();
    }

    void Execute() const;

public:
    static bool CheckForAggroFree(const Player &player, const GW::GamePos &next_pos);
    static bool UpdateMoveState(const Player &player, bool &move_ongoing, const Move &move);
    static bool UpdateMoveState_CallbackAndContinue(const Player &player, const Move &move);
    static bool UpdateMoveState_CastSkill(const Player &player, const Move &move);
    static bool UpdateMoveState_Wait(const Player &player, const Move &move);
    static bool UpdateMoveState_WaitAndStop(const Player &player, const Move &move);
    static bool UpdateMoveState_DistanceLT(const Player &player, const Move &move);

private:
    float x = 0.0F;
    float y = 0.0F;

public:
    GW::GamePos pos;
    std::string name;

    MoveState move_state = MoveState::NO_WAIT_AND_STOP;
    const SkillData *skill_cb = nullptr;
    std::optional<std::function<bool()>> trigger_cb = std::nullopt;
    float dist_threshold = GW::Constants::Range::Compass;
};

template <uint32_t N>
uint32_t GetFirstCloseMove(const Player &player, const std::array<Move, N> &moves)
{
    auto idx = 0U;
    for (const auto move : moves)
    {
        const auto dist_to_move = GW::GetDistance(player.pos, move.pos);
        if (dist_to_move < 500.0F)
            return idx;

        ++idx;
    }

    return idx;
}

template <uint32_t N>
void UpdatedUwMoves_Main(const Player &player, std::array<Move, N> &moves, uint32_t &move_idx, bool &move_ongoing)
{
    static auto trigger_timer_ms = clock();

    if (!move_ongoing)
        return;

    if (move_idx >= moves.size() - 1U)
        return;

    const auto can_be_finished = Move::UpdateMoveState(player, move_ongoing, moves[move_idx]);

    const auto is_moving = player.living->GetIsMoving();
    const auto reached_pos = GamePosCompare(player.pos, moves[move_idx].pos, 0.001F);

    if (!reached_pos && is_moving)
        return;

    const auto state = moves[move_idx].move_state;
    const auto is_proceeding_action = (state != MoveState::NO_WAIT_AND_STOP && state != MoveState::WAIT_AND_STOP);

    if (!reached_pos && !is_moving && can_be_finished)
    {
        static auto last_trigger_time_ms = clock();

        const auto last_trigger_time_diff_ms = TIMER_DIFF(last_trigger_time_ms);
        if (last_trigger_time_diff_ms == 0 || last_trigger_time_diff_ms >= 500)
        {
            last_trigger_time_ms = clock();
            moves[move_idx].Execute();
            Log::Info("Retrigger current move: %s", moves[move_idx].name.data());
            trigger_timer_ms = clock();
        }
        return;
    }

    if (reached_pos && can_be_finished)
    {
        const auto last_trigger_timer_threshold = 500;
        const auto last_trigger_timer_diff = TIMER_DIFF(trigger_timer_ms);
        if (last_trigger_timer_diff < last_trigger_timer_threshold)
            return;
        trigger_timer_ms = clock();

        move_ongoing = false;
        ++move_idx;
        if (is_proceeding_action)
        {
            move_ongoing = true;
            moves[move_idx].Execute();
            Log::Info("Ongoing to next move: %s", moves[move_idx].name.data());
        }
    }
}

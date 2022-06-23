#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string_view>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/GameContainers/GamePos.h>

#include <GuiConstants.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <MathUtils.h>
#include <Player.h>
#include <Skillbars.h>
#include <Types.h>

#include <Timer.h>

#include <imgui.h>

class ActionABC
{
public:
    ActionABC(Player *p, std::string_view t) : player(p), text(t)
    {
    }
    virtual ~ActionABC(){};

    void Draw(const ImVec2 button_size = DEFAULT_BUTTON_SIZE);
    virtual RoutineState Routine() = 0;
    virtual void Update() = 0;

    Player *player = nullptr;
    std::string_view text = nullptr;

    ActionState action_state = ActionState::INACTIVE;
};

class EmoActionABC : public ActionABC
{
public:
    EmoActionABC(Player *p, std::string_view t, EmoSkillbar *s) : ActionABC(p, t), skillbar(s)
    {
    }
    virtual ~EmoActionABC(){};

    EmoSkillbar *skillbar = nullptr;
};

class MesmerActionABC : public ActionABC
{
public:
    MesmerActionABC(Player *p, std::string_view t, MesmerSkillbar *s) : ActionABC(p, t), skillbar(s)
    {
    }
    virtual ~MesmerActionABC(){};

    MesmerSkillbar *skillbar = nullptr;
};

class DbActionABC : public ActionABC
{
public:
    DbActionABC(Player *p, std::string_view t, DbSkillbar *s) : ActionABC(p, t), skillbar(s)
    {
    }
    virtual ~DbActionABC(){};

    DbSkillbar *skillbar = nullptr;
};

enum class MoveState
{
    NONE,
    DONT_WAIT,
    WAIT,
    CAST_SKILL,
};

class Move
{
public:
    // Move and then wait
    Move(const float _x, const float _y, std::string_view _name, const MoveState _move_state)
        : x(_x), y(_y), pos({x, y, 0}), name(_name), move_state(_move_state){};

    // Move, trigger cb, and then wait
    Move(const float _x,
         const float _y,
         std::string_view _name,
         const MoveState _move_state,
         std::function<void()> _trigger_cb)
        : x(_x), y(_y), pos({x, y, 0}), name(_name), trigger_cb(_trigger_cb), move_state(_move_state){};

    // Move, and cast skill at goal
    Move(const float _x,
         const float _y,
         std::string_view _name,
         const MoveState _move_state,
         const SkillData *_skill_cb)
        : x(_x), y(_y), pos({x, y, 0}), name(_name), move_state(_move_state), skill_cb(_skill_cb){};

    // Move, trigger cb, and cast skill at goal
    Move(const float _x,
         const float _y,
         std::string_view _name,
         const MoveState _move_state,
         std::function<void()> _trigger_cb,
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
    static bool UpdateMove(const Player &player, bool &move_ongoing, const Move &move, const Move &next_move);
    static bool UpdateMoveCastSkill(const Player &player, const Move &move);
    static bool UpdateMoveWait(const Player &player, const Move &next_move);

private:
    float x = 0.0F;
    float y = 0.0F;

public:
    GW::GamePos pos;
    std::string_view name;

    MoveState move_state = MoveState::NONE;
    const SkillData *skill_cb = nullptr;
    std::optional<std::function<void()>> trigger_cb = std::nullopt;
};

template <uint32_t N>
uint32_t GetClostestMove(const Player &player, const std::array<Move, N> &moves)
{
    auto closest_move = moves[0];
    auto closest_dist = FLT_MAX;
    auto closest_idx = 0U;
    auto idx = 0U;
    for (const auto move : moves)
    {
        const auto dist_to_move = GW::GetDistance(player.pos, move.pos);
        if (dist_to_move < closest_dist)
        {
            closest_dist = dist_to_move;
            closest_move = move;
            closest_idx = idx;
        }

        ++idx;
    }

    return closest_idx;
}

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
        static auto last_trigger_time_ms = clock();

        const auto last_trigger_time_diff_ms = TIMER_DIFF(last_trigger_time_ms);
        if (last_trigger_time_diff_ms >= 500)
        {
            last_trigger_time_ms = clock();
            moves[move_idx].Execute();
        }
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

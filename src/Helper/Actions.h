#pragma once

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
    Move(const float _x,
         const float _y,
         std::string_view _name,
         const MoveState _moving_state,
         const float _wait_aggro_range = GW::Constants::Range::Spellcast)
        : x(_x), y(_y), pos({x, y, 0}), name(_name), moving_state(_moving_state), wait_aggro_range(_wait_aggro_range){};

    // Move, trigger cb, and then wait
    Move(const float _x,
         const float _y,
         std::string_view _name,
         const MoveState _moving_state,
         std::function<void()> _trigger_cb,
         const float _wait_aggro_range = GW::Constants::Range::Spellcast)
        : x(_x), y(_y), pos({x, y, 0}), name(_name), trigger_cb(_trigger_cb), moving_state(_moving_state),
          wait_aggro_range(_wait_aggro_range){};

    // Move, and cast skill at goal
    Move(const float _x,
         const float _y,
         std::string_view _name,
         const MoveState _moving_state,
         const SkillData *_skill_cb)
        : x(_x), y(_y), pos({x, y, 0}), name(_name), moving_state(_moving_state), skill_cb(_skill_cb){};

    // Move, trigger cb, and cast skill at goal
    Move(const float _x,
         const float _y,
         std::string_view _name,
         const MoveState _moving_state,
         std::function<void()> _trigger_cb,
         const SkillData *_skill_cb)
        : x(_x), y(_y), pos({x, y, 0}), name(_name), trigger_cb(_trigger_cb), moving_state(_moving_state),
          skill_cb(_skill_cb){};

    const char *Name() const
    {
        return name.data();
    }

    void Execute() const;

    static bool CheckForAggroFree(const Player &player, const GW::GamePos &next_pos, const float wait_aggro_range);
    static bool UpdateMove(const Player &player,
                           bool &send_move,
                           const Move &move,
                           const Move &next_move,
                           const float wait_aggro_range);
    static bool UpdateMoveCastSkill(const Player &player, bool &send_move, const Move &move, const Move &next_move);
    static bool UpdateMoveWait(const Player &player,
                               bool &send_move,
                               const Move &next_move,
                               const float wait_aggro_range);

private:
    float x = 0.0;
    float y = 0.0;

public:
    GW::GamePos pos;
    std::string_view name;

    MoveState moving_state = MoveState::NONE;
    const SkillData *skill_cb = nullptr;
    std::optional<std::function<void()>> trigger_cb = std::nullopt;
    float wait_aggro_range = GW::Constants::Range::Spellcast;
};

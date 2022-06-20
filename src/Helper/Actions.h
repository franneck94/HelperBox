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

#include <GuiUtils.h>
#include <Helper.h>
#include <MathUtils.h>
#include <Player.h>
#include <Skillbars.h>
#include <Types.h>

#include <imgui.h>

static const auto ACTIVE_COLOR = ImVec4{0.0F, 200.0F, 0.0F, 80.0F};
static const auto INACTIVE_COLOR = ImVec4{41.0F, 74.0F, 122.0F, 80.0F};
static const auto ON_HOLD_COLOR = ImVec4{255.0F, 226.0F, 0.0F, 80.0F};

static auto COLOR_MAPPING = std::map<uint32_t, ImVec4>{{static_cast<uint32_t>(ActionState::INACTIVE), INACTIVE_COLOR},
                                                       {static_cast<uint32_t>(ActionState::ACTIVE), ACTIVE_COLOR},
                                                       {static_cast<uint32_t>(ActionState::ON_HOLD), ON_HOLD_COLOR}};

class ActionABC
{
public:
    ActionABC(Player *p, const char *const t) : player(p), text(t)
    {
    }
    virtual ~ActionABC(){};

    void Draw(const ImVec2 button_size = DEFAULT_BUTTON_SIZE);
    virtual RoutineState Routine() = 0;
    virtual void Update() = 0;

    Player *player = nullptr;
    const char *const text = nullptr;

    ActionState action_state = ActionState::INACTIVE;
};

class EmoActionABC : public ActionABC
{
public:
    EmoActionABC(Player *p, const char *const t, EmoSkillbar *s) : ActionABC(p, t), skillbar(s)
    {
    }
    virtual ~EmoActionABC(){};

    EmoSkillbar *skillbar = nullptr;
};

class MesmerActionABC : public ActionABC
{
public:
    MesmerActionABC(Player *p, const char *const t, MesmerSkillbar *s) : ActionABC(p, t), skillbar(s)
    {
    }
    virtual ~MesmerActionABC(){};

    MesmerSkillbar *skillbar = nullptr;
};


enum class MoveState
{
    NONE,
    DONT_WAIT_FOR_AGGRO_FREE,
    WAIT_FOR_AGGRO_FREE,
};

class Move
{
public:
    static constexpr size_t NAME_LEN = 140U;

    Move(const float _x,
         const float _y,
         const char *_name,
         std::function<void()> _callback,
         const MoveState _moving_state = MoveState::NONE)
        : x(_x), y(_y), pos({x, y, 0}), name(_name), callback(_callback), moving_state(_moving_state){};

    Move(const float _x, const float _y, const char *_name, const MoveState _moving_state = MoveState::NONE)
        : x(_x), y(_y), pos({x, y, 0}), name(_name), moving_state(_moving_state){};

    const char *Name() const
    {
        return name;
    }

    void Execute();

private:
    float x = 0.0;
    float y = 0.0;

public:
    GW::GamePos pos;
    const char *name;
    MoveState moving_state = MoveState::NONE;
    std::optional<std::function<void()>> callback = std::nullopt;
};

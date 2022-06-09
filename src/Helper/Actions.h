#pragma once

#include <cstdint>
#include <functional>
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

class ActionABC
{
public:
    ActionABC(Player *p, const char *const t) : player(p), text(t)
    {
    }

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

    EmoSkillbar *skillbar = nullptr;
};

class MesmerActionABC : public ActionABC
{
public:
    MesmerActionABC(Player *p, const char *const t, MesmerSkillbar *s) : ActionABC(p, t), skillbar(s)
    {
    }

    MesmerSkillbar *skillbar = nullptr;
};

RoutineState SafeWalk(const GW::GamePos target_position, const bool reset = false);

RoutineState SafeUseSkill(const uint32_t skill_idx, const uint32_t target = 0, const uint32_t call_target = 0);

class Move
{
public:
    static constexpr size_t NAME_LEN = 140U;

    Move(const float _x, const float _y, const char *_name, std::function<void()> _callback)
        : x(_x), y(_y), pos({x, y, 0}), name(_name), callback(_callback){};

    Move(const float _x, const float _y, const char *_name) : x(_x), y(_y), pos({x, y, 0}), name(_name){};

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
    std::optional<std::function<void()>> callback = std::nullopt;
};

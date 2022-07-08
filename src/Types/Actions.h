#pragma once

#include <cstdint>
#include <string_view>

#include <DataPlayer.h>
#include <DataSkillbar.h>
#include <DataSkillbarUw.h>
#include <GuiConstants.h>
#include <Types.h>

class ActionABC
{
public:
    constexpr static auto TIMER_THRESHOLD_MS = uint32_t{200U};

    static bool HasWaitedLongEnough();

    ActionABC(DataPlayer *p, std::string_view t) : player_data(p), text(t){};
    virtual ~ActionABC(){};

    void Draw(const ImVec2 button_size = DEFAULT_BUTTON_SIZE);
    virtual RoutineState Routine() = 0;
    virtual void Update() = 0;

    virtual bool PauseRoutine()
    {
        return false;
    }
    bool ResumeRoutine()
    {
        return !PauseRoutine();
    }

    DataPlayer *player_data;
    std::string_view text;

    ActionState action_state = ActionState::INACTIVE;
};

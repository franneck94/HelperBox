#include <cstdint>
#include <map>

#include <GuiUtils.h>
#include <HelperMaps.h>
#include <Timer.h>

#include "ActionsBase.h"

namespace
{
static auto COLOR_MAPPING = std::map<uint32_t, ImVec4>{{static_cast<uint32_t>(ActionState::INACTIVE), INACTIVE_COLOR},
                                                       {static_cast<uint32_t>(ActionState::ACTIVE), ACTIVE_COLOR},
                                                       {static_cast<uint32_t>(ActionState::ON_HOLD), ON_HOLD_COLOR}};

};

void ActionABC::Draw(const ImVec2 button_size)
{
    if (!IsExplorable())
        action_state = ActionState::INACTIVE;

    const auto color = COLOR_MAPPING[static_cast<uint32_t>(action_state)];
    DrawButton(action_state, color, text, button_size);
}

bool ActionABC::HasWaitedLongEnough()
{
    static auto timer_last_cast_ms = clock();

    const auto last_cast_diff_ms = TIMER_DIFF(timer_last_cast_ms);
    if (last_cast_diff_ms < TIMER_THRESHOLD_MS)
        return false;

    timer_last_cast_ms = clock();
    return true;
}

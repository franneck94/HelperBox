#include <set>
#include <vector>

#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>
#include <GWCA/Managers/ItemMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Packets/StoC.h>

#include <GuiUtils.h>
#include <Helper.h>
#include <Timer.h>
#include <UwHelper.h>

#include "Actions.h"

void ActionABC::Draw(const ImVec2 button_size)
{
    if (!IsExplorable())
        action_state = ActionState::INACTIVE;

    const auto color = COLOR_MAPPING[static_cast<uint32_t>(action_state)];
    DrawButton(action_state, color, text, button_size);
}

bool HasWaitedEnough()
{
    static auto timer_last_cast_ms = clock();

    const auto last_cast_diff_ms = TIMER_DIFF(timer_last_cast_ms);
    if (last_cast_diff_ms < 150)
        return false;

    timer_last_cast_ms = clock();
    return true;
}

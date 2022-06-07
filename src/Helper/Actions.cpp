#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>
#include <GWCA/Managers/ItemMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Packets/StoC.h>

#include <fmt/format.h>

#include <Logger.h>

#include "Actions.h"

void ActionABC::Draw(const ImVec2 button_size)
{
    if (!IsExplorable())
        action_state = ActionState::INACTIVE;

    const auto color = COLOR_MAPPING[static_cast<uint32_t>(action_state)];
    DrawButton(action_state, color, text, button_size);
}

RoutineState SafeWalk(const GW::GamePos target_position, const bool reset)
{
    static auto map_zoned = false;
    static auto state = RoutineState::NONE;
    ResetState(state);
    if (reset)
        state = RoutineState::NONE;

    const auto me = GW::Agents::GetPlayer();
    if (!me)
        return state;

    const auto current_position = me->pos;
    const auto reached = GamePosCompare(target_position, current_position, 0.001F);

    if (!reached)
    {
        if (RoutineState::NONE == state)
        {
            const auto message = fmt::format("Traveling to coords {} {}", target_position.x, target_position.y);
            Log::Info(message.data());
            GW::Agents::Move(target_position);
            state = RoutineState::ACTIVE;
        }
    }
    else
    {
        state = RoutineState::FINISHED;
    }

    return state;
}

RoutineState SafeUseSkill(const uint32_t skill_idx, const uint32_t target, const uint32_t call_target)
{
    if (target != 0 && call_target != 0)
        GW::GameThread::Enqueue([&]() { GW::SkillbarMgr::UseSkill(skill_idx, target, call_target); });
    else if (target != 0)
        GW::GameThread::Enqueue([&]() { GW::SkillbarMgr::UseSkill(skill_idx, target); });
    else
        GW::GameThread::Enqueue([&]() { GW::SkillbarMgr::UseSkill(skill_idx); });

    return RoutineState::FINISHED;
}

void Move::Execute()
{
    if (!CanMove())
        return;

    const auto me = GW::Agents::GetPlayer();
    if (!me)
        return;

    GW::Agents::Move(x, y);
    Log::Info("Moving to (%.0f, %.0f)", x, y);

    if (callback.has_value())
    {
        Log::Info("Calling the callback");
        callback.value()();
    }
}

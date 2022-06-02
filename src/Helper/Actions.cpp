#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>
#include <GWCA/Managers/ItemMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Packets/StoC.h>

#include <fmt/format.h>

#include "Actions.h"

RoutineState SafeWalk(GW::GamePos target_position, const bool reset)
{
    static auto map_zoned = false;
    static auto state = RoutineState::NONE;
    ResetState(state);
    if (reset)
        state = RoutineState::NONE;

    if (!map_zoned && GW::Constants::InstanceType::Loading == GW::Map::GetInstanceType())
    {
        map_zoned = true;
    }

    if (!IsMapReady())
    {
        return state;
    }

    if (map_zoned || DetectPlayerIsDead())
    {
        map_zoned = false;

        state = RoutineState::FINISHED;
        return state;
    }

    const auto me = GW::Agents::GetPlayer();

    if (nullptr == me)
    {
        return state;
    }

    auto current_position = me->pos;
    const auto reached = GamePosCompare(target_position, current_position, 0.001F);

    if (!reached)
    {
        if (RoutineState::NONE == state)
        {
            Log::Info(fmt::format("Traveling to coords {} {}", target_position.x, target_position.y).data());

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
    if (!IsMapReady())
    {
        return RoutineState::NONE;
    }

    const auto me = GW::Agents::GetPlayer();

    if (nullptr == me)
    {
        return RoutineState::NONE;
    }

    if (target != 0 && call_target != 0)
    {
        GW::GameThread::Enqueue([&]() { GW::SkillbarMgr::UseSkill(skill_idx, target, call_target); });
    }
    else if (target != 0)
    {
        GW::GameThread::Enqueue([&]() { GW::SkillbarMgr::UseSkill(skill_idx, target); });
    }
    else
    {
        GW::GameThread::Enqueue([&]() { GW::SkillbarMgr::UseSkill(skill_idx); });
    }

    return RoutineState::FINISHED;
}

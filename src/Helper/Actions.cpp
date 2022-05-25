#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>
#include <GWCA/Managers/ItemMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Packets/StoC.h>

#include <fmt/format.h>

#include "Actions.h"

RoutineState SafeTravel(const GW::Constants::MapID target_map,
                        const GW::Constants::MapRegion target_region,
                        const GW::Constants::MapLanguage target_language)
{
    static auto state = RoutineState::NONE;
    ResetState(state);

    if (!IsMapReady())
    {
        return state;
    }

    if (DetectPlayerIsDead())
    {
        return state;
    }

    const auto current_map = GW::Map::GetMapID();
    const auto current_region = static_cast<GW::Constants::MapRegion>(GW::Map::GetRegion());
    const auto current_language = static_cast<GW::Constants::MapLanguage>(GW::Map::GetLanguage());

    const bool travel_finished =
        ((target_map == current_map) && (current_region == target_region) && (current_language == target_language));

    if (!travel_finished)
    {
        if (RoutineState::NONE == state)
        {
            Log::Info(fmt::format("Traveling to map").data());

            GW::Map::Travel(target_map, 0, static_cast<int>(target_region), static_cast<int>(target_language));

            state = RoutineState::ACTIVE;
        }
    }
    else if (travel_finished && !IsLoading())
    {
        state = RoutineState::FINISHED;
    }

    return state;
}

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

RoutineState SafeLoadSkillTemplate(std::string_view code)
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

    GW::GameThread::Enqueue([&]() { GW::SkillbarMgr::LoadSkillTemplate(code.data()); });
    Log::Info(fmt::format("Loaded template: {}", code).data());

    return RoutineState::FINISHED;
}

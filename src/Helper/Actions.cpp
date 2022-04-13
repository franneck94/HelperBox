#include <GWCA/Managers/GameThreadMgr.h>
#include <GWCA/Managers/ItemMgr.h>

#include <fmt/format.h>

#include "Actions.h"

ActionState SafeTravel(const GW::Constants::MapID target_map,
                       const GW::Constants::MapRegion target_region,
                       const GW::Constants::MapLanguage target_language)
{
    static auto state = ActionState::NONE;
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
        if (ActionState::NONE == state)
        {
            Log::Info(fmt::format("Traveling to map").data());

            GW::Map::Travel(target_map, 0, static_cast<int>(target_region), static_cast<int>(target_language));

            state = ActionState::ACTIVE;
        }
    }
    else if (travel_finished && !IsLoading())
    {
        state = ActionState::FINISHED;
    }

    return state;
}

ActionState SafeWalk(GW::GamePos target_position)
{
    static auto map_zoned = false;
    static auto state = ActionState::NONE;
    ResetState(state);

    if (GW::Constants::InstanceType::Loading == GW::Map::GetInstanceType())
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

        state = ActionState::FINISHED;
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
        if (ActionState::NONE == state)
        {
            Log::Info(fmt::format("Traveling to coords {} {}", target_position.x, target_position.y).data());

            GW::Agents::Move(target_position);

            state = ActionState::ACTIVE;
        }
    }
    else
    {
        state = ActionState::FINISHED;
    }

    return state;
}

ActionState SafeUseSkill(const uint32_t skill_idx, const uint32_t target, const uint32_t call_target)
{
    if (!IsMapReady())
    {
        return ActionState::NONE;
    }

    const auto me = GW::Agents::GetPlayer();

    if (nullptr == me)
    {
        return ActionState::NONE;
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

    return ActionState::FINISHED;
}

ActionState SafeLoadSkillTemplate(std::string_view code)
{
    if (!IsMapReady())
    {
        return ActionState::NONE;
    }

    const auto me = GW::Agents::GetPlayer();

    if (nullptr == me)
    {
        return ActionState::NONE;
    }

    GW::GameThread::Enqueue([&]() { GW::SkillbarMgr::LoadSkillTemplate(code.data()); });
    Log::Info(fmt::format("Loaded template: {}", code).data());

    return ActionState::FINISHED;
}

ActionState SafeChangeTarget(const TargetType type)
{
    static auto state = ActionState::NONE;
    ResetState(state);

    if (!IsMapReady())
    {
        return ActionState::NONE;
    }

    const auto me = GW::Agents::GetPlayer();

    if (nullptr == me)
    {
        return ActionState::NONE;
    }

    if (TargetType::Item == type)
    {
        TargetNearest(type, GW::Constants::Range::Nearby);
    }
    else
    {
        TargetNearest(type);
    }

    return ActionState::FINISHED;
}

ActionState SafeGotoTarget()
{
    static auto state = ActionState::NONE;
    ResetState(state);

    if (!IsMapReady())
    {
        return ActionState::NONE;
    }

    const auto me = GW::Agents::GetPlayer();

    if (nullptr == me)
    {
        return ActionState::NONE;
    }

    const GW::Agent *const target = GW::Agents::GetTarget();

    if (!target || target->type != 0x200)
    {
        return ActionState::FINISHED;
    }

    auto target_position = target->pos;
    const auto reached = GamePosCompare(target_position, me->pos, 0.001F);

    if (!reached)
    {
        if (ActionState::NONE == state)
        {
            Log::Info(fmt::format("Traveling to target {} {}", target_position.x, target_position.y).data());

            GW::Agents::Move(target_position);

            state = ActionState::ACTIVE;
        }
    }
    else
    {
        state = ActionState::FINISHED;
    }

    return state;
}

ActionState SafeOpenChest()
{
    static auto state = ActionState::NONE;
    ResetState(state);

    if (!IsMapReady())
    {
        return ActionState::NONE;
    }

    const auto me = GW::Agents::GetPlayer();

    if (nullptr == me)
    {
        return ActionState::NONE;
    }

    const auto target = GW::Agents::GetTarget();

    if (nullptr == target)
    {
        return ActionState::FINISHED;
    }

    GW::Agents::GoSignpost(target);
    GW::Items::OpenLockedChest();

    return ActionState::FINISHED;
}

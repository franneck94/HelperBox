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
        Wait(10);
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

RoutineState SafeChangeTarget(const TargetType type)
{
    static auto state = RoutineState::NONE;
    ResetState(state);

    if (!IsMapReady())
    {
        return RoutineState::NONE;
    }

    const auto me = GW::Agents::GetPlayer();

    if (nullptr == me)
    {
        return RoutineState::NONE;
    }

    if (TargetType::Item == type)
    {
        TargetNearest(type, GW::Constants::Range::Compass / 2.0F);
    }
    else
    {
        TargetNearest(type);
    }

    return RoutineState::FINISHED;
}

RoutineState SafeGotoTarget()
{
    static auto state = RoutineState::NONE;
    ResetState(state);

    if (!IsMapReady())
    {
        return RoutineState::NONE;
    }

    const auto me = GW::Agents::GetPlayer();

    if (nullptr == me)
    {
        return RoutineState::NONE;
    }

    const GW::Agent *const target = GW::Agents::GetTarget();

    if (!target || target->type != 0x200)
    {
        return RoutineState::FINISHED;
    }

    auto target_position = target->pos;
    const auto reached = GamePosCompare(target_position, me->pos, 0.001F);

    if (!reached)
    {
        if (RoutineState::NONE == state)
        {
            Log::Info(fmt::format("Traveling to target {} {}", target_position.x, target_position.y).data());

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

RoutineState SafeOpenChest()
{
    static auto state = RoutineState::NONE;
    ResetState(state);

    if (!IsMapReady())
    {
        return RoutineState::NONE;
    }

    const auto me = GW::Agents::GetPlayer();

    if (nullptr == me)
    {
        return RoutineState::NONE;
    }

    const auto target = GW::Agents::GetTarget();

    if (nullptr == target)
    {
        return RoutineState::FINISHED;
    }

    if (target && target->type == 0x200)
    {
        GW::Agents::GoSignpost(target);
        GW::Items::OpenLockedChest();
    }

    return RoutineState::FINISHED;
}

RoutineState SafeResign(bool issue_resign)
{
    static bool send_package = false;
    static RoutineState state = RoutineState::NONE;
    ResetState(state);

    if (!IsMapReady())
    {
        return RoutineState::FINISHED;
    }

    if (issue_resign && state != RoutineState::ACTIVE)
    {
        GW::Chat::SendChat('/', "resign");
        state = RoutineState::ACTIVE;
        send_package = false;
    }

    if (!GW::PartyMgr::GetIsPartyDefeated())
    {
        return RoutineState::FINISHED;
    }

    if (!send_package && state == RoutineState::ACTIVE)
    {
        GW::CtoS::SendPacket(0x4, GAME_CMSG_PARTY_RETURN_TO_OUTPOST);
        send_package = true;
        return RoutineState::FINISHED;
    }

    return state;
}

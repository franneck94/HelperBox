#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>
#include <GWCA/Managers/SkillbarMgr.h>

#include <Helper.h>
#include <Logger.h>
#include <MathUtils.h>
#include <Skillbars.h>
#include <Types.h>

#include <fmt/format.h>

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

bool CastBondIfNotAvailable(const SkillData &skill_data, const uint32_t target_id, const PlayerData *const player_data)
{
    const auto has_bond = AgentHasBuff(static_cast<GW::Constants::SkillID>(skill_data.id), target_id);
    const auto bond_avail = skill_data.CanBeCasted(player_data->energy);

    if (!has_bond && bond_avail)
    {
        GW::GameThread::Enqueue([&]() { GW::SkillbarMgr::UseSkill(skill_data.idx, target_id); });
        return true;
    }
    return false;
}

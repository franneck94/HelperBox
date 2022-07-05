#include <algorithm>
#include <set>

#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/ChatMgr.h>

#include <AgentData.h>
#include <Helper.h>
#include <HelperAgents.h>
#include <HelperUw.h>
#include <HelperUwPos.h>
#include <Logger.h>
#include <PlayerData.h>
#include <Timer.h>

#include <fmt/format.h>

#include "Move.h"

void MoveABC::Execute() const
{
    const auto me = GW::Agents::GetPlayer();
    if (!CanMove() || !me)
        return;

    const auto dist = GW::GetDistance(me->pos, pos);
    if (dist > 5000.0F)
    {
        Log::Info("Too far away!");
        return;
    }

    if (x != 0.0F && y != 0.0F)
    {
        GW::Agents::Move(x, y);
        Log::Info("%s - Moving to (%.0f, %.0f)", name.data(), x, y);
    }

    if (cb_fn.has_value())
        cb_fn.value()();
}

bool Move_CastSkillABC::UpdateMoveState(const PlayerData &player_data, const AgentLivingData *, bool &move_ongoing)
{
    move_ongoing = true;

    static auto started_cast = false;
    static auto timer = clock();

    if (player_data.living->GetIsMoving())
        timer = clock();

    const auto reached_pos = GamePosCompare(player_data.pos, pos, 0.1F);
    if (reached_pos && started_cast && !player_data.living->GetIsMoving() && !player_data.living->GetIsCasting())
        started_cast = false;

    const auto skill_data = GW::SkillbarMgr::GetSkillConstantData(skill_cb->id);
    if (!skill_data)
        return true;
    const auto cast_time_s = (skill_data->activation * 1.0F) * 1000.0F;
    const auto timer_diff = TIMER_DIFF(timer);

    if (reached_pos && !started_cast && timer_diff > 300)
    {
        started_cast = true;
        if (skill_cb->recharge > 0)
        {
            started_cast = false;
            return true;
        }

        skill_cb->Cast(player_data.energy);
        return false;
    }

    if (timer_diff < 300)
        return false;

    const auto is_casting = player_data.living->GetIsCasting();
    const auto timer_exceeded = timer_diff > cast_time_s;
    const auto wait = (timer_exceeded || is_casting);

    if (started_cast && wait)
        return false;

    started_cast = false;
    return true;
}

bool Move_WaitABC::UpdateMoveState(const PlayerData &player_data,
                                   const AgentLivingData *agents_data,
                                   bool &move_ongoing)
{
    move_ongoing = true;

    static auto canceled_move = false;
    const auto aggro_free = CheckForAggroFree(player_data, agents_data, pos);
    if (aggro_free)
    {
        canceled_move = false;
        return true;
    }

    if (!canceled_move && player_data.living->GetIsMoving())
    {
        canceled_move = true;
        Log::Info("Canceled Movement based on aggro");
        CancelMovement();
        return false;
    }

    return false;
}

bool Move_DistanceABC::UpdateMoveState(const PlayerData &player_data, const AgentLivingData *, bool &move_ongoing)
{
    move_ongoing = true;

    const auto lt_id = GetTankId();
    if (!lt_id)
        return false;

    const auto lt_agent = GW::Agents::GetAgentByID(lt_id);
    if (!lt_agent)
        return false;

    const auto dist = GW::GetDistance(player_data.pos, lt_agent->pos);
    if (dist < dist_threshold)
        return false;

    return true;
}

bool Move_NoWaitABC::UpdateMoveState(const PlayerData &, const AgentLivingData *, bool &move_ongoing)
{
    move_ongoing = true;
    return true;
}

bool Move_PositionABC::UpdateMoveState(const PlayerData &, const AgentLivingData *, bool &move_ongoing)
{
    move_ongoing = true;

    if (!trigger_agent)
        return false;

    if (IsNearToGamePos(trigger_pos, trigger_agent->pos, trigger_threshold))
        return true;

    return false;
}

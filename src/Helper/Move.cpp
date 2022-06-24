#include <set>

#include <GWCA/Managers/ChatMgr.h>

#include <Helper.h>
#include <UwHelper.h>

#include <Logger.h>
#include <Timer.h>

#include <fmt/format.h>

#include "Move.h"

void Move::Execute() const
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

    if (trigger_cb.has_value())
        trigger_cb.value()();
}


bool Move::CheckForAggroFree(const Player &player, const GW::GamePos &next_pos)
{
    const auto filter_ids =
        std::set<uint32_t>{GW::Constants::ModelID::UW::SkeletonOfDhuum1, GW::Constants::ModelID::UW::SkeletonOfDhuum2};

    const auto livings = GetEnemiesInAggro(player);
    const auto result_ids_Aggro = FilterAgentIDS(livings, filter_ids);

    if (player.pos.x == next_pos.x && player.pos.y == next_pos.y)
        return result_ids_Aggro.size() == 0;
    else if (result_ids_Aggro.size() == 1)
        return false;

    const auto rect = GameRectangle(player.pos, next_pos, GW::Constants::Range::Spellcast);
    const auto filtered_livings = GetEnemiesInGameRectangle(rect);

    const auto is_near_chamber_skele = IsAtChamberSkele(player);
    const auto is_right_at_chamber_skele = IsRightAtChamberSkele(player);
    const auto is_in_chamber_where_to_move = (is_near_chamber_skele && !is_right_at_chamber_skele);

    const auto is_near_to_at_vale_start = IsAtValeStart(player);
    const auto is_near_to_vale_house = IsAtValeHouse(player);
    const auto is_right_at_vale_house = IsRightAtValeHouse(player);
    const auto is_in_vale_where_to_move =
        ((is_near_to_at_vale_start || is_near_to_vale_house) && !is_right_at_vale_house);

    auto result_ids_rect = std::set<uint32_t>{};
    if (is_in_chamber_where_to_move || is_in_vale_where_to_move) // ignore skeles here
        result_ids_rect = FilterAgentIDS(filtered_livings, filter_ids);
    else
        result_ids_rect = FilterAgentIDS(filtered_livings, std::set<uint32_t>{});

    return result_ids_rect.size() == 0;
}

bool Move::UpdateMoveState_CastSkill(const Player &player, const Move &move)
{
    static auto started_cast = false;
    static auto timer = clock();

    if (player.living->GetIsMoving())
        timer = clock();

    const auto reached_pos = GamePosCompare(player.pos, move.pos, 0.001F);
    if (reached_pos && started_cast && !player.living->GetIsMoving() && !player.living->GetIsCasting())
        started_cast = false;

    const auto skill_data = GW::SkillbarMgr::GetSkillConstantData(move.skill_cb->id);
    const auto cast_time_s = (skill_data.activation * 1.0F) * 1000.0F;
    const auto timer_diff = TIMER_DIFF(timer);

    if (reached_pos && !started_cast && move.skill_cb && timer_diff > 200)
    {
        started_cast = true;
        if (move.skill_cb->recharge > 0)
        {
            started_cast = false;
            return true;
        }

        move.skill_cb->Cast(player.energy);
        return false;
    }

    if (timer_diff < 200)
        return false;

    const auto is_casting = player.living->GetIsCasting();
    const auto timer_exceeded = timer_diff > cast_time_s;
    const auto wait = (timer_exceeded || is_casting);

    if (started_cast && wait)
        return false;

    started_cast = false;
    return true;
}

bool Move::UpdateMoveState_Wait(const Player &player, const Move &move)
{
    static auto canceled_move = false;

    const auto aggro_free = Move::CheckForAggroFree(player, move.pos);
    if (aggro_free)
    {
        canceled_move = false;
        return true;
    }

    if (!canceled_move && player.living->GetIsMoving())
    {
        canceled_move = true;
        Log::Info("Canceled Movement based on aggro");
        GW::CtoS::SendPacket(0x4, GAME_CMSG_CANCEL_MOVEMENT);
        return false;
    }

    return false;
}

bool Move::UpdateMoveState_WaitAndStop(const Player &player, const Move &move)
{
    return UpdateMoveState_Wait(player, move);
}

bool Move::UpdateMoveState_DistanceLT(const Player &player, const Move &move)
{
    const auto lt_id = GetTankId();
    if (!lt_id)
        return false;

    const auto lt_agent = GW::Agents::GetAgentByID(lt_id);
    if (!lt_agent)
        return false;

    const auto dist_threshold = 3600.0F;
    const auto dist = GW::GetDistance(player.pos, lt_agent->pos);
    if (dist < dist_threshold)
        return false;

    return UpdateMoveState_Wait(player, move);
}

bool Move::UpdateMoveState(const Player &player, bool &move_ongoing, const Move &move)
{
    move_ongoing = true;

    switch (move.move_state)
    {
    case MoveState::CAST_SKILL_AND_CONTINUE:
    {
        return Move::UpdateMoveState_CastSkill(player, move);
    }
    case MoveState::WAIT_AND_CONTINUE:
    {
        return Move::UpdateMoveState_Wait(player, move);
    }
    case MoveState::DISTANCE_AND_CONTINUE:
    {
        return Move::UpdateMoveState_DistanceLT(player, move);
    }
    case MoveState::WAIT_AND_STOP:
    {
        return Move::UpdateMoveState_WaitAndStop(player, move);
    }
    case MoveState::NO_WAIT_AND_CONTINUE:
    case MoveState::NO_WAIT_AND_STOP:
    default:
    {
        return true;
    }
    }
}

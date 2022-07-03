#include <set>

#include <GWCA/Managers/ChatMgr.h>

#include <AgentLivingData.h>
#include <Helper.h>
#include <PlayerData.h>
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

    if (this->move_state != MoveState::CALLBACK_AND_CONTINUE && trigger_cb.has_value())
        trigger_cb.value()();
}

bool Move::IsAtFilterSkelePos(const PlayerData &player_data, const GW::GamePos &next_pos)
{
    const auto is_near_chamber_skele = IsAtChamberSkele(player_data.pos);
    const auto is_right_at_chamber_skele = IsRightAtChamberSkele(player_data.pos);
    const auto is_in_chamber_where_to_move = (is_near_chamber_skele && !is_right_at_chamber_skele);

    const auto is_near_to_at_vale_start = IsAtValeStart(player_data.pos);
    const auto is_near_to_vale_house = IsAtValeHouse(player_data.pos);
    const auto is_right_at_vale_house = IsRightAtValeHouse(player_data.pos);
    const auto is_in_vale_where_to_move =
        ((is_near_to_at_vale_start || is_near_to_vale_house) && !is_right_at_vale_house);

    const auto is_at_basement_stair = GW::GetDistance(next_pos, GW::GamePos{-6263.33F, 9899.79F, 0}) < 1280.0F;
    const auto is_to_basement1 = GW::GetDistance(next_pos, GW::GamePos{-5183.64F, 8876.31F, 0}) < 1280.0F;

    return (is_in_chamber_where_to_move || is_in_vale_where_to_move || is_to_basement1 || is_at_basement_stair);
}

bool Move::CheckForAggroFree(const PlayerData &player_data,
                             const AgentLivingData *agents_data,
                             const GW::GamePos &next_pos)
{
    if (!agents_data)
        return true;

    const auto filter_ids =
        std::set<uint32_t>{GW::Constants::ModelID::UW::SkeletonOfDhuum1, GW::Constants::ModelID::UW::SkeletonOfDhuum2};

    const auto livings = GetEnemiesInRange(player_data, GW::Constants::Range::Earshot);
    const auto result_ids_Aggro = FilterAgentIDS(livings, filter_ids);

    if (player_data.pos.x == next_pos.x && player_data.pos.y == next_pos.y)
        return result_ids_Aggro.size() == 0;
    else if (result_ids_Aggro.size() == 1)
        return false;

    const auto rect = GameRectangle(player_data.pos, next_pos, GW::Constants::Range::Spellcast);
    const auto filtered_livings = GetEnemiesInGameRectangle(rect, agents_data->enemies);

    const auto move_pos_is_right_at_spirits1 = GW::GetDistance(next_pos, GW::GamePos{-13760.19F, 358.15F, 0}) < 1280.0F;

    auto result_ids_rect = std::set<uint32_t>{};
    if (IsAtFilterSkelePos(player_data, next_pos)) // ignore skeles here
    {
        result_ids_rect = FilterAgentIDS(filtered_livings, filter_ids);
    }
    else if (move_pos_is_right_at_spirits1) // ignore spirits here
    {
        const auto player_pos = player_data.pos;
        auto enemies = agents_data->enemies;
        if (enemies.size() == 0)
            return true;

        std::sort(enemies.begin(), enemies.end(), [&player_pos](const auto a1, const auto a2) {
            const auto sqrd1 = GW::GetDistance(player_pos, a1->pos);
            const auto sqrd2 = GW::GetDistance(player_pos, a2->pos);
            return sqrd1 < sqrd2;
        });

        const auto dist = GW::GetDistance(player_pos, enemies[0]->pos);
        return dist > 3000.0F;
    }
    else
    {
        result_ids_rect = FilterAgentIDS(filtered_livings, std::set<uint32_t>{});
    }

    return result_ids_rect.size() == 0;
}

bool Move::UpdateMoveState_CallbackAndContinue(const PlayerData &player_data, const Move &move)
{
    static auto executed_callback_successful = false;

    const auto reached_pos = GamePosCompare(player_data.pos, move.pos, 0.1F);
    if (reached_pos && executed_callback_successful && !player_data.living->GetIsMoving())
        executed_callback_successful = false;

    if (reached_pos && !executed_callback_successful)
    {
        if (move.trigger_cb.has_value())
            executed_callback_successful = move.trigger_cb.value()();
        return false;
    }

    executed_callback_successful = false;
    return true;
}

bool Move::UpdateMoveState_CastSkill(const PlayerData &player_data, const Move &move)
{
    static auto started_cast = false;
    static auto timer = clock();

    if (player_data.living->GetIsMoving())
        timer = clock();

    const auto reached_pos = GamePosCompare(player_data.pos, move.pos, 0.1F);
    if (reached_pos && started_cast && !player_data.living->GetIsMoving() && !player_data.living->GetIsCasting())
        started_cast = false;

    const auto skill_data = GW::SkillbarMgr::GetSkillConstantData(move.skill_cb->id);
    if (!skill_data)
        return true;
    const auto cast_time_s = (skill_data->activation * 1.0F) * 1000.0F;
    const auto timer_diff = TIMER_DIFF(timer);

    if (reached_pos && !started_cast && timer_diff > 300)
    {
        started_cast = true;
        if (move.skill_cb->recharge > 0)
        {
            started_cast = false;
            return true;
        }

        move.skill_cb->Cast(player_data.energy);
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

bool Move::UpdateMoveState_Wait(const PlayerData &player_data, const AgentLivingData *agents_data, const Move &move)
{
    static auto canceled_move = false;

    const auto aggro_free = Move::CheckForAggroFree(player_data, agents_data, move.pos);
    if (aggro_free)
    {
        canceled_move = false;
        return true;
    }

    if (!canceled_move && player_data.living->GetIsMoving())
    {
        canceled_move = true;
        Log::Info("Canceled Movement based on aggro");
        GW::CtoS::SendPacket(0x4, GAME_CMSG_CANCEL_MOVEMENT);
        return false;
    }

    return false;
}

bool Move::UpdateMoveState_DistanceLT(const PlayerData &player_data, const Move &move)
{
    const auto lt_id = GetTankId();
    if (!lt_id)
        return false;

    const auto lt_agent = GW::Agents::GetAgentByID(lt_id);
    if (!lt_agent)
        return false;

    const auto dist_threshold = move.dist_threshold;
    const auto dist = GW::GetDistance(player_data.pos, lt_agent->pos);
    if (dist < dist_threshold)
        return false;

    return true;
}

bool Move::UpdateMoveState(const PlayerData &player_data,
                           const AgentLivingData *agents_data,
                           bool &move_ongoing,
                           const Move &move)
{
    move_ongoing = true;

    switch (move.move_state)
    {
    case MoveState::CAST_SKILL_AND_CONTINUE:
    {
        return Move::UpdateMoveState_CastSkill(player_data, move);
    }
    case MoveState::WAIT_AND_CONTINUE:
    case MoveState::WAIT_AND_STOP:
    {
        return Move::UpdateMoveState_Wait(player_data, agents_data, move);
    }
    case MoveState::DISTANCE_AND_CONTINUE:
    {
        return Move::UpdateMoveState_DistanceLT(player_data, move);
    }
    case MoveState::CALLBACK_AND_CONTINUE:
    {
        return Move::UpdateMoveState_CallbackAndContinue(player_data, move);
    }
    case MoveState::NO_WAIT_AND_CONTINUE:
    case MoveState::NO_WAIT_AND_STOP:
    default:
    {
        return true;
    }
    }
}

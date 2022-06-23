#include <set>
#include <vector>

#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>
#include <GWCA/Managers/ItemMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Packets/StoC.h>

#include <UwHelper.h>

#include <Logger.h>
#include <Timer.h>

#include <fmt/format.h>

#include "Actions.h"

void ActionABC::Draw(const ImVec2 button_size)
{
    if (!IsExplorable())
        action_state = ActionState::INACTIVE;

    const auto color = COLOR_MAPPING[static_cast<uint32_t>(action_state)];
    DrawButton(action_state, color, text, button_size);
}

void Move::Execute() const
{
    if (!CanMove())
        return;

    const auto me = GW::Agents::GetPlayer();
    if (!me)
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
    {
        Log::Info("Calling the callback");
        trigger_cb.value()();
    }
}

bool Move::CheckForAggroFree(const Player &player, const GW::GamePos &next_pos)
{
    const auto filter_ids =
        std::set<uint32_t>{GW::Constants::ModelID::UW::SkeletonOfDhuum1, GW::Constants::ModelID::UW::SkeletonOfDhuum2};

    if (player.pos.x == next_pos.x && player.pos.y == next_pos.y)
    {
        const auto livings = GetEnemiesInAggro(player);
        const auto result_ids = FilterAgentIDS(livings, filter_ids);

        if (result_ids.size() == 0)
            return true;
        return false;
    }

    const auto rect = GameRectangle(player.pos, next_pos, GW::Constants::Range::Spellcast);
    const auto filtered_livings = GetEnemiesInGameRectangle(rect);

    const auto is_at_chamber_skele = IsAtChamberSkele(&player);
    const auto is_in_vale = IsSomewhereInVale(&player);

    auto result_ids = std::set<uint32_t>{};
    if (is_at_chamber_skele || is_in_vale)
        result_ids = FilterAgentIDS(filtered_livings, std::set<uint32_t>{});
    else
        result_ids = FilterAgentIDS(filtered_livings, filter_ids);

    if (result_ids.size() == 0)
        return true;
    return false;
}

bool Move::UpdateMoveCastSkill(const Player &player, const Move &move)
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

        const auto ret = move.skill_cb->Cast(player.energy);
        if (ret != RoutineState::FINISHED)
            Log::Info("Casting %u unsuccessful", skill_data.skill_id);
        else
            Log::Info("Casting %u successful", skill_data.skill_id);
        return false;
    }
    else if (timer_diff < 200)
        return false;

    const auto is_casting = player.living->GetIsCasting();
    const auto timer_exceeded = timer_diff > cast_time_s;
    const auto wait = (timer_exceeded || is_casting);

    if (started_cast && wait)
        return false;

    started_cast = false;

    return true;
}

bool Move::UpdateMoveWait(const Player &player, const Move &next_move)
{
    static auto canceled_move = false;

    const auto aggro_free = Move::CheckForAggroFree(player, next_move.pos);
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

    static auto timer_start = clock();
    const auto timer_diff_ms = TIMER_DIFF(timer_start);
    if (timer_diff_ms >= 5000)
    {
        GW::Chat::WriteChat(GW::Chat::Channel::CHANNEL_GROUP, L"Waiting...");
        timer_start = clock();
    }

    return false;
}

bool Move::UpdateMove(const Player &player, bool &move_ongoing, const Move &move, const Move &next_move)
{
    move_ongoing = true;

    switch (move.move_state)
    {
    case MoveState::CAST_SKILL:
    {
        return Move::UpdateMoveCastSkill(player, move);
    }
    case MoveState::WAIT:
    {
        return Move::UpdateMoveWait(player, next_move);
    }
    case MoveState::DONT_WAIT:
    case MoveState::NONE:
    default:
    {
        return true;
    }
    }
}

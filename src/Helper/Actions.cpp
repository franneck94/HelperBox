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
    const auto is_at_vale_house = IsAtValeHouse(&player);

    auto result_ids = std::set<uint32_t>{};
    if (is_at_chamber_skele || is_at_vale_house)
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

    if (!started_cast && move.skill_cb)
    {
        started_cast = true;
        move.skill_cb->Cast(player.energy);
        return false;
    }

    const auto timer_diff = TIMER_DIFF(timer);
    const auto is_casting = player.living->GetIsCasting();
    const auto skill_data = GW::SkillbarMgr::GetSkillConstantData(move.skill_cb->id);
    const auto timer_exceeded = timer_diff < (skill_data.activation * 0.80F * 1000.0F);
    const auto wait = (timer_exceeded || is_casting);

    if (started_cast && wait)
        return false;

    if (started_cast)
    {
        started_cast = false;
        timer = clock();
    }

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

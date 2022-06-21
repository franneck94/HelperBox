#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>
#include <GWCA/Managers/ItemMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Packets/StoC.h>

#include <fmt/format.h>

#include <Logger.h>
#include <Timer.h>

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

bool Move::CheckForAggroFree(const Player &player, const GW::GamePos &next_pos, const float wait_aggro_range)
{
    const auto agents_array = GW::Agents::GetAgentArray();

    const auto ids = std::array<uint32_t, 0>{};
    const auto filter_at_agent = std::array<uint32_t, 2>{GW::Constants::ModelID::UW::SkeletonOfDhuum1,
                                                         GW::Constants::ModelID::UW::SkeletonOfDhuum2};
    const auto filter_at_target = std::array<uint32_t, 5>{GW::Constants::ModelID::UW::SkeletonOfDhuum1,
                                                          GW::Constants::ModelID::UW::SkeletonOfDhuum2,
                                                          GW::Constants::ModelID::UW::TorturedSpirit1,
                                                          GW::Constants::ModelID::UW::TorturedSpirit,
                                                          2372U};

    auto agents_at_player = std::vector<GW::AgentLiving *>{};
    FilterAgentsAtPositionWithDistance(player.pos,
                                       agents_array,
                                       agents_at_player,
                                       ids,
                                       filter_at_agent,
                                       GW::Constants::Allegiance::Enemy,
                                       wait_aggro_range);

    auto agents_at_intermediate_pos1 = std::vector<GW::AgentLiving *>{};
    const auto intermediate_pos1 =
        GW::GamePos{(player.pos.x + next_pos.x) / 2.0F, (player.pos.y + next_pos.y) / 2.0F, player.pos.zplane};
    FilterAgentsAtPositionWithDistance(intermediate_pos1,
                                       agents_array,
                                       agents_at_intermediate_pos1,
                                       ids,
                                       filter_at_target,
                                       GW::Constants::Allegiance::Enemy,
                                       GW::Constants::Range::Spellcast);

    auto agents_at_target_pos = std::vector<GW::AgentLiving *>{};
    FilterAgentsAtPositionWithDistance(next_pos,
                                       agents_array,
                                       agents_at_target_pos,
                                       ids,
                                       filter_at_target,
                                       GW::Constants::Allegiance::Enemy,
                                       wait_aggro_range);

    if (agents_at_player.size() == 0 && agents_at_intermediate_pos1.size() == 0 && agents_at_target_pos.size() == 0)
        return true;

    return false;
}

bool Move::UpdateMoveCastSkill(const Player &player, bool &send_move, const Move &move, const Move &next_move)
{
    static auto started_cast = false;
    static auto timer = clock();

    if (!started_cast && move.skill_cb)
    {
        timer = clock();
        started_cast = true;
        move.skill_cb->Cast(player.energy);
        return false;
    }

    const auto timer_diff = TIMER_DIFF(timer);
    const auto is_casting = player.living->GetIsCasting();
    const auto skill_data = GW::SkillbarMgr::GetSkillConstantData(move.skill_cb->id);
    const auto timer_exceeded = timer_diff < (skill_data.activation * 1000.0F);
    const auto wait = (timer_exceeded || is_casting);

    if (started_cast && wait)
        return false;

    if (started_cast)
    {
        started_cast = false;
        timer = clock();
        send_move = true;
        next_move.Execute();
    }

    return true;
}

bool Move::UpdateMoveWait(const Player &player, bool &send_move, const Move &next_move, const float wait_aggro_range)
{
    const auto aggro_free = Move::CheckForAggroFree(player, next_move.pos, wait_aggro_range);
    if (aggro_free)
    {
        send_move = true;
        next_move.Execute();
        return true;
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

bool Move::UpdateMove(const Player &player,
                      bool &send_move,
                      const Move &move,
                      const Move &next_move,
                      const float wait_aggro_range)
{
    switch (move.moving_state)
    {
    case MoveState::CAST_SKILL:
    {
        return Move::UpdateMoveCastSkill(player, send_move, move, next_move);
    }
    case MoveState::DONT_WAIT:
    {
        send_move = true;
        next_move.Execute();
        return true;
    }
    case MoveState::WAIT:
    {
        return Move::UpdateMoveWait(player, send_move, next_move, wait_aggro_range);
    }
    case MoveState::NONE:
    default:
    {
        send_move = false;
        return true;
    }
    }
}

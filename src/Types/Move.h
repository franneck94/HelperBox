#pragma once

#include <functional>
#include <map>
#include <optional>
#include <string>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/GameContainers/GamePos.h>

#include <Logger.h>
#include <Timer.h>

#include <AgentData.h>
#include <Helper.h>
#include <PlayerData.h>
#include <SkillbarData.h>
#include <Types.h>

class MoveABC
{
public:
    MoveABC(const float _x,
            const float _y,
            std::string_view _name,
            std::optional<std::function<bool()>> _cb_fn = std::nullopt)
        : x(_x), y(_y), pos({x, y, 0}), name(_name), cb_fn(_cb_fn){};
    virtual ~MoveABC() noexcept {};

    const char *Name() const
    {
        return name.data();
    }

    void Execute() const;

    virtual bool UpdateMoveState(const PlayerData &player_data,
                                 const AgentLivingData *agents_data,
                                 bool &move_ongoing) = 0;

    template <uint32_t N>
    static void UpdatedUwMoves(const PlayerData &player_data,
                               const AgentLivingData *agents_data,
                               std::array<MoveABC *, N> &moves,
                               uint32_t &move_idx,
                               bool &move_ongoing)
    {
        static auto trigger_timer_ms = clock();
        static auto already_reached_pos = false;

        if (!move_ongoing)
            return;

        if (move_idx >= moves.size() - 1U)
            return;

        const auto can_be_finished = moves[move_idx]->UpdateMoveState(player_data, agents_data, move_ongoing);

        const auto is_moving = player_data.living->GetIsMoving();
        const auto reached_pos = GamePosCompare(player_data.pos, moves[move_idx]->pos, 0.001F);
        if (!already_reached_pos && reached_pos)
            already_reached_pos = true;

        if (!already_reached_pos && is_moving)
            return;

        if (!already_reached_pos && !is_moving && can_be_finished)
        {
            static auto last_trigger_time_ms = clock();

            const auto last_trigger_time_diff_ms = TIMER_DIFF(last_trigger_time_ms);
            if (last_trigger_time_diff_ms == 0 || last_trigger_time_diff_ms >= MoveABC::last_trigger_timer_threshold_ms)
            {
                last_trigger_time_ms = clock();
                moves[move_idx]->Execute();
                Log::Info("Retrigger current move: %s", moves[move_idx]->name.data());
                trigger_timer_ms = clock();
            }
            return;
        }

        if (already_reached_pos)
        {
            const auto last_trigger_timer_diff = TIMER_DIFF(trigger_timer_ms);
            if (last_trigger_timer_diff < MoveABC::last_trigger_timer_threshold_ms)
                return;
            trigger_timer_ms = clock();

            already_reached_pos = false;
            move_ongoing = false;
            const auto proceed = moves[move_idx]->is_proceeding_move;
            ++move_idx;
            const auto trigger = moves[move_idx]->is_not_distance_based;
            if (proceed)
            {
                move_ongoing = true;
                if (trigger)
                {
                    moves[move_idx]->Execute();
                    Log::Info("Ongoing to next move: %s", moves[move_idx]->name.data());
                }
                else
                {
                    Log::Info("Waiting for distance...");
                }
            }
        }
    }

    template <uint32_t N>
    static uint32_t GetFirstCloseMove(const PlayerData &player_data, const std::array<MoveABC *, N> &moves)
    {
        auto idx = 0U;
        for (const auto move : moves)
        {
            const auto dist_to_move = GW::GetDistance(player_data.pos, move->pos);
            if (dist_to_move < GW::Constants::Range::Spellcast)
                return idx;

            ++idx;
        }

        return 0U;
    }

private:
    float x = 0.0F;
    float y = 0.0F;

public:
    GW::GamePos pos;
    std::string name;
    std::optional<std::function<bool()>> cb_fn = std::nullopt;
    bool is_proceeding_move = false;
    bool is_not_distance_based = true;

    static const long last_trigger_timer_threshold_ms = 500L;
};

class Move_NoWaitABC : public MoveABC
{
public:
    Move_NoWaitABC(const float _x,
                   const float _y,
                   const std::string &_name,
                   std::optional<std::function<bool()>> _cb_fn = std::nullopt)
        : MoveABC(_x, _y, _name, _cb_fn)
    {
        is_proceeding_move = false;
    };
    ~Move_NoWaitABC(){};

    bool UpdateMoveState(const PlayerData &player_data,
                         const AgentLivingData *agents_data,
                         bool &move_ongoing) override;
};

class Move_NoWaitAndContinue : public Move_NoWaitABC
{
public:
    Move_NoWaitAndContinue(const float _x,
                           const float _y,
                           const std::string &_name,
                           std::optional<std::function<bool()>> _cb_fn = std::nullopt)
        : Move_NoWaitABC(_x, _y, _name, _cb_fn)
    {
        is_proceeding_move = true;
    };
    ~Move_NoWaitAndContinue(){};
};

class Move_NoWaitAndStop : public Move_NoWaitABC
{
public:
    Move_NoWaitAndStop(const float _x,
                       const float _y,
                       const std::string &_name,
                       std::optional<std::function<bool()>> _cb_fn = std::nullopt)
        : Move_NoWaitABC(_x, _y, _name, _cb_fn)
    {
        is_proceeding_move = false;
    };
    ~Move_NoWaitAndStop(){};
};

class Move_WaitABC : public MoveABC
{
public:
    Move_WaitABC(const float _x,
                 const float _y,
                 const std::string &_name,
                 std::optional<std::function<bool()>> _cb_fn = std::nullopt)
        : MoveABC(_x, _y, _name, _cb_fn){};
    ~Move_WaitABC(){};

    bool UpdateMoveState(const PlayerData &player_data,
                         const AgentLivingData *agents_data,
                         bool &move_ongoing) override;
};

class Move_WaitAndContinue : public Move_WaitABC
{
public:
    Move_WaitAndContinue(const float _x,
                         const float _y,
                         const std::string &_name,
                         std::optional<std::function<bool()>> _cb_fn = std::nullopt)
        : Move_WaitABC(_x, _y, _name, _cb_fn)
    {
        is_proceeding_move = true;
    };
    ~Move_WaitAndContinue(){};
};

class Move_WaitAndStop : public Move_WaitABC
{
public:
    Move_WaitAndStop(const float _x,
                     const float _y,
                     const std::string &_name,
                     std::optional<std::function<bool()>> _cb_fn = std::nullopt)
        : Move_WaitABC(_x, _y, _name, _cb_fn)
    {
        is_proceeding_move = false;
    };
    ~Move_WaitAndStop(){};

    bool UpdateMoveState(const PlayerData &player_data,
                         const AgentLivingData *agents_data,
                         bool &move_ongoing) override;
};

class Move_CastSkillABC : public MoveABC
{
public:
    Move_CastSkillABC(const float _x,
                      const float _y,
                      const std::string &_name,
                      const SkillData *_skill_cb,
                      std::optional<std::function<bool()>> _cb_fn = std::nullopt)
        : MoveABC(_x, _y, _name, _cb_fn), skill_cb(_skill_cb){};
    ~Move_CastSkillABC(){};

    bool UpdateMoveState(const PlayerData &player_data,
                         const AgentLivingData *agents_data,
                         bool &move_ongoing) override;

    const SkillData *skill_cb = nullptr;
};

class Move_CastSkillAndContinue : public Move_CastSkillABC
{
public:
    Move_CastSkillAndContinue(const float _x,
                              const float _y,
                              const std::string &_name,
                              const SkillData *_skill_cb,
                              std::optional<std::function<bool()>> _cb_fn = std::nullopt)
        : Move_CastSkillABC(_x, _y, _name, _skill_cb, _cb_fn)
    {
        is_proceeding_move = true;
    };
    ~Move_CastSkillAndContinue(){};
};

class Move_CastSkillAndStop : public Move_CastSkillABC
{
public:
    Move_CastSkillAndStop(const float _x,
                          const float _y,
                          const std::string &_name,
                          const SkillData *_skill_cb,
                          std::optional<std::function<bool()>> _cb_fn = std::nullopt)
        : Move_CastSkillABC(_x, _y, _name, _skill_cb, _cb_fn)
    {
        is_proceeding_move = true;
    };
    ~Move_CastSkillAndStop(){};
};

class Move_DistanceABC : public MoveABC
{
public:
    Move_DistanceABC(const float _x,
                     const float _y,
                     const std::string &_name,
                     const float _dist_threshold,
                     std::optional<std::function<bool()>> _cb_fn = std::nullopt)
        : MoveABC(_x, _y, _name, _cb_fn), dist_threshold(_dist_threshold)
    {
        is_not_distance_based = false;
    };
    ~Move_DistanceABC(){};

    bool UpdateMoveState(const PlayerData &player_data,
                         const AgentLivingData *agents_data,
                         bool &move_ongoing) override;

    float dist_threshold;
};

class Move_DistanceAndContinue : public Move_DistanceABC
{
public:
    Move_DistanceAndContinue(const float _x,
                             const float _y,
                             const std::string &_name,
                             const float _dist_threshold,
                             std::optional<std::function<bool()>> _cb_fn = std::nullopt)
        : Move_DistanceABC(_x, _y, _name, _dist_threshold, _cb_fn)
    {
        is_proceeding_move = true;
    };
};

class Move_DistanceAndStop : public Move_DistanceABC
{
public:
    Move_DistanceAndStop(const float _x,
                         const float _y,
                         const std::string &_name,
                         const float _dist_threshold,
                         std::optional<std::function<bool()>> _cb_fn = std::nullopt)
        : Move_DistanceABC(_x, _y, _name, _dist_threshold, _cb_fn)
    {
        is_proceeding_move = false;
    };
};

class Move_PositionABC : public MoveABC
{
public:
    Move_PositionABC(const float _x,
                     const float _y,
                     const std::string &_name,
                     const GW::GamePos &_trigger_pos,
                     const GW::Agent *_trigger_agent,
                     std::optional<std::function<bool()>> _cb_fn = std::nullopt)
        : MoveABC(_x, _y, _name, _cb_fn), trigger_pos(_trigger_pos), trigger_agent(_trigger_agent)
    {
        is_not_distance_based = false;
    };
    ~Move_PositionABC(){};

    bool UpdateMoveState(const PlayerData &player_data,
                         const AgentLivingData *agents_data,
                         bool &move_ongoing) override;

    GW::GamePos trigger_pos;
    const GW::Agent *trigger_agent = nullptr;
};

class Move_PositionAndContinue : public Move_PositionABC
{
public:
    Move_PositionAndContinue(const float _x,
                             const float _y,
                             const std::string &_name,
                             const GW::GamePos &_trigger_pos,
                             const GW::Agent *_trigger_agent,
                             std::optional<std::function<bool()>> _cb_fn = std::nullopt)
        : Move_PositionABC(_x, _y, _name, _trigger_pos, _trigger_agent, _cb_fn)
    {
        is_proceeding_move = true;
    };
    ~Move_PositionAndContinue(){};
};

class Move_PositionAndStop : public Move_PositionABC
{
public:
    Move_PositionAndStop(const float _x,
                         const float _y,
                         const std::string &_name,
                         const GW::GamePos &_trigger_pos,
                         const GW::Agent *_trigger_agent,
                         std::optional<std::function<bool()>> _cb_fn = std::nullopt)
        : Move_PositionABC(_x, _y, _name, _trigger_pos, _trigger_agent, _cb_fn)
    {
        is_proceeding_move = false;
    };
    ~Move_PositionAndStop(){};
};

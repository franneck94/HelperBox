#pragma once

#include <cstdint>

enum class ActionState
{
    INACTIVE = 0,
    ACTIVE = 1,
    ON_HOLD = 2,
};

ActionState StateNegation(const ActionState state);

void StateOnActive(ActionState &state);

void StateOnHold(ActionState &state);

enum class TargetType
{
    Gadget = 0,
    PlayerData,
    Npc,
    Item,
    Living_Ally,
    Living_Enemy,
    Living_Npc,
};

enum class RoutineState
{
    NONE = 0,
    ACTIVE = 1,
    FINISHED = 2
};

void ResetState(RoutineState &state);

struct PlayerMapping
{
    uint32_t id;
    uint32_t party_idx;
};

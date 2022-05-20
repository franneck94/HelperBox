#pragma once

enum class ModuleState
{
    INACTIVE = 0,
    ACTIVE = 1,
    ON_HOLD = 2,
};

ModuleState StateNegation(const ModuleState state);

void StateOnActive(ModuleState &state);

void StateOnHold(ModuleState &state);

enum class TargetType
{
    Gadget = 0,
    Player,
    Npc,
    Item,
    Living
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

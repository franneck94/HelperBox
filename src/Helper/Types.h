#pragma once

enum class ModuleState
{
    INACTIVE = 0,
    ACTIVE = 1,
    ON_HOLD = 2,
};

ModuleState StateNegation(const ModuleState state);

void StateOnHoldToggle(ModuleState &state);

enum class TargetType
{
    Gadget = 0,
    Player,
    Npc,
    Item,
    Living
};

enum class ActionState
{
    NONE = 0,
    ACTIVE = 1,
    FINISHED = 2
};

void ResetState(ActionState &state);

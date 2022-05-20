#include "Types.h"

ModuleState StateNegation(const ModuleState state)
{
    switch (state)
    {
    case ModuleState::INACTIVE:
    {
        return ModuleState::ACTIVE;
        break;
    }
    case ModuleState::ON_HOLD:
    {
        return ModuleState::ON_HOLD;
        break;
    }
    case ModuleState::ACTIVE:
    default:
    {
        return ModuleState::INACTIVE;
        break;
    }
    }
}

void ResetState(RoutineState &state)
{
    if (RoutineState::FINISHED == state)
    {
        state = RoutineState::NONE;
    }
}

void StateOnActive(ModuleState &state)
{
    if (state == ModuleState::ON_HOLD)
    {
        state = ModuleState::ACTIVE;
    }
}

void StateOnHold(ModuleState &state)
{
    if (state == ModuleState::ACTIVE)
    {
        state = ModuleState::ON_HOLD;
    }
}

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

void ResetState(ActionState &state)
{
    if (ActionState::FINISHED == state)
    {
        state = ActionState::NONE;
    }
}

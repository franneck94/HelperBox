#include "Types.h"

ActionState StateNegation(const ActionState state)
{
    switch (state)
    {
    case ActionState::INACTIVE:
    {
        return ActionState::ACTIVE;
        break;
    }
    case ActionState::ON_HOLD:
    case ActionState::ACTIVE:
    default:
    {
        return ActionState::INACTIVE;
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

void StateOnActive(ActionState &state)
{
    if (state == ActionState::ON_HOLD)
    {
        state = ActionState::ACTIVE;
    }
}

void StateOnHold(ActionState &state)
{
    if (state == ActionState::ACTIVE)
    {
        state = ActionState::ON_HOLD;
    }
}

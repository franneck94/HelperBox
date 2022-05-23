#pragma once

#include "Actions.h"
#include "Player.h"

class ActionBaseClass
{
public:
    ActionBaseClass(Player *p, char *const t) : player(p), text(t)
    {
    }

    void Draw();
    virtual RoutineState Routine() = 0;
    virtual void Update() = 0;

    Player *player = nullptr;
    ActionState action_state = ActionState::INACTIVE;
    char *const text;
};

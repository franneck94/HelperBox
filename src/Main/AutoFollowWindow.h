#pragma once

#include "stdafx.h"

#include <cstdint>

#include <GWCA/GameEntities/Agent.h>

#include <Actions.h>
#include <Player.h>
#include <Types.h>

#include <Base/HelperBoxWindow.h>

class AutoFollowAction : public ActionABC
{
public:
    AutoFollowAction(Player *p) : ActionABC(p, "Follow")
    {
    }

    RoutineState Routine() override;
    void Update() override;
};

class AutoFollowWindow : public HelperBoxWindow
{
public:
    AutoFollowWindow() : player({}), auto_follow(&player){};
    ~AutoFollowWindow(){};

    static AutoFollowWindow &Instance()
    {
        static AutoFollowWindow instance;
        return instance;
    }

    const char *Name() const override
    {
        return "AutoFollowWindow";
    }

    void Draw(IDirect3DDevice9 *pDevice) override;
    void Update(float delta) override;

private:
    Player player;
    AutoFollowAction auto_follow;
};

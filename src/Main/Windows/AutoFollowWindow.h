#pragma once

#include "stdafx.h"

#include <cstdint>

#include <GWCA/GameEntities/Agent.h>

#include <Actions.h>
#include <PlayerData.h>
#include <Types.h>

#include <Base/HelperBoxWindow.h>

class AutoFollowAction : public ActionABC
{
public:
    AutoFollowAction(PlayerData *p) : ActionABC(p, "Follow")
    {
    }

    RoutineState Routine() override;
    void Update() override;
};

class AutoFollowWindow : public HelperBoxWindow
{
public:
    AutoFollowWindow() : player_data({}), auto_follow(&player_data){};
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
    void Update(float delta, const PlayerData &, const AgentLivingData &) override;

private:
    PlayerData player_data;
    AutoFollowAction auto_follow;
};

#pragma once

#include "stdafx.h"

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <string_view>

#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Packets/Opcodes.h>

#include <Timer.h>

#include <Coloring.h>
#include <GuiUtils.h>
#include <Player.h>
#include <Types.h>

#include <Base/HelperBoxWindow.h>

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

class Pumping : public ActionBaseClass
{
public:
    Pumping(Player *p) : ActionBaseClass(p, "Pumping")
    {
    }

    RoutineState Routine() override;
    void Update() override;
};

class TankBonding : public ActionBaseClass
{
public:
    TankBonding(Player *p) : ActionBaseClass(p, "Tank Bonds")
    {
    }

    RoutineState Routine() override;
    void Update() override;
};

class PlayerBonding : public ActionBaseClass
{
public:
    PlayerBonding(Player *p) : ActionBaseClass(p, "Player Bonds")
    {
    }

    RoutineState Routine() override;
    void Update() override;
};

class FusePull : public ActionBaseClass
{
public:
    static constexpr auto MIN_FUSE_PULL_RANGE = float{1200.0F};
    static constexpr auto MAX_FUSE_PULL_RANGE = float{1248.0F};
    static constexpr auto FUSE_PULL_RANGE = float{1220.0F};

    FusePull(Player *p) : timer(TIMER_INIT()), ActionBaseClass(p, "Fuse Pull")
    {
    }

    RoutineState Routine();
    void Update();

    void ResetData()
    {
        GW::CtoS::SendPacket(0x4, GAME_CMSG_CANCEL_MOVEMENT);
        timer = TIMER_INIT();
        routine_state = RoutineState::NONE;
        requested_pos = GW::GamePos{};
        stuck_counter = 0;
        step = 0;
    }

    clock_t timer;
    RoutineState routine_state = RoutineState::NONE;
    GW::GamePos requested_pos = GW::GamePos{};
    uint32_t stuck_counter = 0;
    uint32_t step = 0;
};

class EmoWindow : public HelperBoxWindow
{
public:
    EmoWindow() : player(), fuse_pull(&player), pumping(&player), tank_bonding(&player), player_bonding(&player){};
    ~EmoWindow(){};

    static EmoWindow &Instance()
    {
        static EmoWindow instance;
        return instance;
    }

    const char *Name() const override
    {
        return "EmoWindow";
    }

    void Initialize() override
    {
        HelperBoxWindow::Initialize();
    }

    void LoadSettings(CSimpleIni *ini) override
    {
        HelperBoxWindow::LoadSettings(ini);
        show_menubutton = true;
    }

    void SaveSettings(CSimpleIni *ini) override
    {
        HelperBoxWindow::SaveSettings(ini);
    }

    void Draw(IDirect3DDevice9 *pDevice) override;
    void Update(float delta) override;

private:
    void EmoSkillRoutine();
    bool EmoBondTankRoutine();
    bool EmoBondPlayerRoutine();
    bool EmoFusePull();

    Player player;
    FusePull fuse_pull;
    Pumping pumping;
    TankBonding tank_bonding;
    PlayerBonding player_bonding;
};

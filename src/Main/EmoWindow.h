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

#include <Player.h>
#include <Types.h>

#include <HelperBoxWindow.h>

class EmoAction
{
public:
    EmoAction(Player *p, char *const t) : player(p), text(t)
    {
    }

    void Draw();
    virtual bool Routine() = 0;
    virtual void Update() = 0;

    Player *player = nullptr;
    ModuleState state = ModuleState::INACTIVE;
    char *const text;
};

class Pumping : public EmoAction
{
public:
    Pumping(Player *p) : EmoAction(p, "Pumping")
    {
    }

    bool Routine() override;
    void Update() override;
};

class TankBonding : public EmoAction
{
public:
    TankBonding(Player *p) : EmoAction(p, "Tank Bonds")
    {
    }

    bool Routine() override;
    void Update() override;
};

class PlayerBonding : public EmoAction
{
public:
    PlayerBonding(Player *p) : EmoAction(p, "Player Bonds")
    {
    }

    bool Routine() override;
    void Update() override;
};

class FusePull : public EmoAction
{
public:
    static constexpr auto MIN_FUSE_PULL_RANGE = float{1200.0F};
    static constexpr auto MAX_FUSE_PULL_RANGE = float{1248.0F};
    static constexpr auto FUSE_PULL_RANGE = float{1220.0F};

    FusePull(Player *p) : timer(TIMER_INIT()), EmoAction(p, "Fuse Pull")
    {
    }

    bool Routine();
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

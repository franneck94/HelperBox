#pragma once

#include "stdafx.h"

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <string_view>

#include <Timer.h>

#include <Player.h>
#include <Types.h>

#include <HelperBoxWindow.h>

class Pumping
{
public:
    static constexpr auto casting_button_text = "Pumping";

    Pumping(Player *p) : player(p)
    {
    }

    void Draw();
    void Routine();
    void Update();

    Player *player = nullptr;
};

class TankBonding
{
public:
    static constexpr auto bonding_tank_button_text = "Tank Bonds";

    TankBonding(Player *p) : player(p)
    {
    }

    void Draw();
    bool Routine();
    void Update();

    Player *player = nullptr;
};

class PlayerBonding
{
public:
    static constexpr auto bonding_player_button_text = "Player Bonds";

    PlayerBonding(Player *p) : player(p)
    {
    }

    void Draw();
    bool Routine();
    void Update();

    Player *player = nullptr;
};

class FusePull
{
public:
    static constexpr auto fuse_pull_button_text = "Fuse Pull";
    static constexpr auto MIN_FUSE_PULL_RANGE = float{1200.0F};
    static constexpr auto MAX_FUSE_PULL_RANGE = float{1248.0F};
    static constexpr auto FUSE_PULL_RANGE = float{1220.0F};

    FusePull(Player *p) : timer(TIMER_INIT()), player(p)
    {
    }

    void Draw();
    bool Routine();
    void Update();

    void ResetData()
    {
        timer = TIMER_INIT();
        state = ActionState::NONE;
        requested_pos = GW::GamePos{};
        last_pos = GW::GamePos{};
        stuck_counter = 0;
        step = 0;
    }

    clock_t timer;
    ActionState state = ActionState::NONE;
    GW::GamePos requested_pos = GW::GamePos{};
    GW::GamePos last_pos = GW::GamePos{};
    uint32_t stuck_counter = 0;
    uint32_t step = 0;

    Player *player = nullptr;
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

    void Initialize() override;
    void Draw(IDirect3DDevice9 *pDevice) override;
    void Update(float delta) override;
    void LoadSettings(CSimpleIni *ini) override;
    void SaveSettings(CSimpleIni *ini) override;

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

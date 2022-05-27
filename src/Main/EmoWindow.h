#pragma once

#include "stdafx.h"

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <string_view>

#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/StoCMgr.h>
#include <GWCA/Packets/Opcodes.h>
#include <GWCA/Packets/StoC.h>
#include <GWCA/Utilities/Hook.h>

#include <Timer.h>

#include <Actions.h>
#include <GuiUtils.h>
#include <Player.h>
#include <Types.h>

#include <Base/HelperBoxWindow.h>

class Pumping : public EmoActionABC
{
public:
    Pumping(Player *p, EmoSkillbar *s) : EmoActionABC(p, "Pumping", s)
    {
        GW::StoC::RegisterPacketCallback<GW::Packet::StoC::AgentAdd>(
            &Summon_AgentAdd_Entry,
            [&](GW::HookStatus *, GW::Packet::StoC::AgentAdd *pak) -> void {
                if (pak->type != 1)
                    return;

                if (GW::Map::GetMapID() != GW::Constants::MapID::The_Underworld)
                    return;

                uint32_t player_number = (pak->agent_type ^ 0x20000000);

                if (player_number != 514) // Turtle id
                    return;

                Log::Info("Summoned turtle");
                found_turtle = true;
                turtle_id = pak->agent_id;
            });
    }

    RoutineState Routine() override;
    void Update() override;

    GW::HookEntry Summon_AgentAdd_Entry;
    bool found_turtle = false;
    uint32_t turtle_id = 0;
};

class TankBonding : public EmoActionABC
{
public:
    TankBonding(Player *p, EmoSkillbar *s) : EmoActionABC(p, "Tank Bonds", s)
    {
    }

    RoutineState Routine() override;
    void Update() override;
};

class PlayerBonding : public EmoActionABC
{
public:
    PlayerBonding(Player *p, EmoSkillbar *s) : EmoActionABC(p, "Player Bonds", s)
    {
    }

    RoutineState Routine() override;
    void Update() override;
};

class FusePull : public EmoActionABC
{
public:
    static constexpr auto MIN_FUSE_PULL_RANGE = float{1200.0F};
    static constexpr auto MAX_FUSE_PULL_RANGE = float{1248.0F};
    static constexpr auto FUSE_PULL_RANGE = float{1220.0F};

    FusePull(Player *p, EmoSkillbar *s) : timer(TIMER_INIT()), EmoActionABC(p, "Fuse Pull", s)
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
    EmoWindow()
        : player({}), skillbar({}), fuse_pull(&player, &skillbar), pumping(&player, &skillbar),
          tank_bonding(&player, &skillbar), player_bonding(&player, &skillbar)
    {
        if (skillbar.ValidateData())
        {
            skillbar.Load();
        }
    };
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
    EmoSkillbar skillbar;

    FusePull fuse_pull;
    Pumping pumping;
    TankBonding tank_bonding;
    PlayerBonding player_bonding;
};

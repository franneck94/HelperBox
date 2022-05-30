#pragma once

#include "stdafx.h"

#include <cstdint>
#include <cstring>
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

class Move
{
public:
    Move(const float _x, const float _y, const float _range, const char *_name)
        : x(_x), y(_y), range(_range), pos({x, y, 0})
    {
        strncpy(name, _name, 140);
    };

    float x = 0.0;
    float y = 0.0;
    float range = 0.0;
    GW::GamePos pos;
    char name[140] = "Move";

    const char *Name() const
    {
        return name;
    }

    void Execute();
};

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
    GW::HookEntry OnDialog_Entry;
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
    EmoWindow();
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

    uint32_t move_idx = 0;
    std::vector<Move> moves;

    GW::HookEntry MapLoaded_Entry;
};

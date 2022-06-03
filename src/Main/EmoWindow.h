#pragma once

#include "stdafx.h"

#include <array>
#include <cstdint>

#include <GWCA/Packets/StoC.h>
#include <GWCA/Utilities/Hook.h>

#include <Actions.h>
#include <Callbacks.h>
#include <GuiUtils.h>
#include <Player.h>
#include <Timer.h>
#include <Types.h>

#include <Base/HelperBoxWindow.h>

class Pumping : public EmoActionABC
{
public:
    Pumping(Player *p, EmoSkillbar *s);

    RoutineState Routine() override;
    void Update() override;

private:
    RoutineState RoutineSelfBonds();
    RoutineState RoutineLT();
    RoutineState RoutineTurtle();
    RoutineState RoutineWisdom();
    RoutineState RoutineGDW();
    RoutineState RoutinePI(const uint32_t dhuum_id);

    void WarnDistanceLT();

    GW::HookEntry Summon_AgentAdd_Entry;
    bool found_turtle = false;
    uint32_t turtle_id = 0;

    GW::Agent *lt_agent = nullptr;

    bool interrupted = false;
    GW::HookEntry GenericValue_Entry;
};

class TankBonding : public EmoActionABC
{
public:
    TankBonding(Player *p, EmoSkillbar *s) : EmoActionABC(p, "Tank Bonds", s)
    {
        GW::StoC::RegisterPacketCallback<GW::Packet::StoC::GenericValue>(
            &GenericValue_Entry,
            [this](GW::HookStatus *status, GW::Packet::StoC::GenericValue *packet) -> void {
                UNREFERENCED_PARAMETER(status);
                if (action_state == ActionState::ACTIVE && SkillStoppedCallback(packet, player))
                    interrupted = true;
            });
    }

    RoutineState Routine() override;
    void Update() override;

    bool interrupted = false;
    GW::HookEntry GenericValue_Entry;
};

class PlayerBonding : public EmoActionABC
{
public:
    PlayerBonding(Player *p, EmoSkillbar *s) : EmoActionABC(p, "Player Bonds", s)
    {
        GW::StoC::RegisterPacketCallback<GW::Packet::StoC::GenericValue>(
            &GenericValue_Entry,
            [this](GW::HookStatus *status, GW::Packet::StoC::GenericValue *packet) -> void {
                UNREFERENCED_PARAMETER(status);
                if (action_state == ActionState::ACTIVE && SkillStoppedCallback(packet, player))
                    interrupted = true;
            });
    }

    RoutineState Routine() override;
    void Update() override;

    bool interrupted = false;
    GW::HookEntry GenericValue_Entry;
};

class FuseRange : public EmoActionABC
{
public:
    static constexpr auto FUSE_PULL_RANGE = float{1220.0F};

    FuseRange(Player *p, EmoSkillbar *s) : EmoActionABC(p, "Fuse Range", s)
    {
    }

    RoutineState Routine();
    void Update();

    void ResetData()
    {
        routine_state = RoutineState::NONE;
        requested_pos = GW::GamePos{};
    }

    RoutineState routine_state = RoutineState::NONE;
    GW::GamePos requested_pos = GW::GamePos{};
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
    bool ActivationConditions();

    void EmoSkillRoutine();
    bool EmoBondTankRoutine();
    bool EmoBondPlayerRoutine();
    bool EmoFuseRange();

    Player player;
    EmoSkillbar skillbar;

    FuseRange fuse_pull;
    Pumping pumping;
    TankBonding tank_bonding;
    PlayerBonding player_bonding;

    uint32_t move_idx = 0;
    std::array<Move, 26> moves = {Move{1248.0F, 6965.509766F, 5000.0F, "Spawn"},
                                  Move{-583.28F, 9275.68F, 5000.0F, "Lab Stairs1"},
                                  Move{-2730.79F, 10159.21F, 5000.0F, "Lab Stairs2"},
                                  Move{-5683.589844F, 12662.990234F, 5000.0F, "Lab Reaper"},
                                  Move{-6459.410156F, 9943.219727F, 5000.0F, "Fuse Pull 1"},
                                  Move{-6241.24F, 7945.73F, 5000.0F, "Basement"},
                                  Move{-8763.36F, 5551.18F, 5000.0F, "Basement Stairs"},
                                  Move{-7980.55F, 4308.90F, 5000.0F, "Fuse Pull 2"},
                                  Move{-8764.08F, 2156.60F, 5000.0F, "Vale Entry"},
                                  Move{-12264.129883F, 1821.180054F, 5000.0F, "Vale House"},
                                  Move{-13872.34F, 2332.34F, 5000.0F, "Spirits 1"},
                                  Move{-13760.19F, 358.15F, 5000.0F, "Spirits 2"},
                                  Move{-12145.44F, 1101.74F, 5000.0F, "Spirits 3"},
                                  Move{-8764.08F, 2156.60F, 5000.0F, "Vale Entry"},
                                  Move{-7980.55F, 4308.90F, 5000.0F, "Basement Stairs"},
                                  Move{-6241.24F, 7945.73F, 5000.0F, "Basement"},
                                  Move{-5683.589844F, 12662.990234F, 5000.0F, "Lab Reaper"},
                                  Move{-6035.58F, 11274.30F, 5000.0F, "Keeper 1/2"},
                                  Move{-3881.71F, 11280.04F, 5000.0F, "Keeper 3"},
                                  Move{-1502.45F, 9737.64F, 5000.0F, "Keeper 4/5"},
                                  Move{-266.03F, 9304.26F, 5000.0F, "Lab Stairs1"},
                                  Move{1207.05F, 7732.16F, 5000.0F, "Keeper 6"},
                                  Move{1354.31F, 10063.58F, 5000.0F, "To Wastes 1"},
                                  Move{3489.18F, 8177.49F, 5000.0F, "To Wastes 2"},
                                  Move{5385.25F, 8866.17F, 5000.0F, "To Wastes 3"},
                                  Move{6022.19F, 11318.40F, 5000.0F, "To Wastes 3"}};

    GW::HookEntry MapLoaded_Entry;
    GW::HookEntry GenericValue_Entry;
};

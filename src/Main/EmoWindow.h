#pragma once

#include "stdafx.h"

#include <array>
#include <cstdint>

#include <GWCA/Packets/StoC.h>
#include <GWCA/Utilities/Hook.h>

#include <Actions.h>
#include <Callbacks.h>
#include <GuiUtils.h>
#include <Logger.h>
#include <Player.h>
#include <Timer.h>
#include <Types.h>

#include <Base/HelperBoxWindow.h>

class Pumping : public EmoActionABC
{
public:
    Pumping(Player *p, EmoSkillbar *s, uint32_t *_bag_idx, uint32_t *_start_slot_idx);

    RoutineState Routine() override;
    void Update() override;

private:
    RoutineState RoutineSelfBonds();
    RoutineState RoutineCanthaGuards();
    RoutineState RoutineLT();
    RoutineState RoutineTurtle();
    RoutineState RoutineWisdom();
    RoutineState RoutineGDW();
    RoutineState RoutinePI(const uint32_t dhuum_id);

    void WarnDistanceLT();

    static auto constexpr cantha_ids = std::array<uint32_t, 4>{8990U, 8991U, 8992U, 8993U};

    uint32_t *bag_idx = nullptr;
    uint32_t *start_slot_idx = nullptr;

    GW::HookEntry Summon_AgentAdd_Entry;
    bool found_turtle = false;
    uint32_t turtle_id = 0;

    GW::Agent *lt_agent = nullptr;
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
    static constexpr auto FUSE_PULL_RANGE = float{1230.0F};

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
        bag_idx = ini->GetLongValue(Name(), VAR_NAME(bag_idx), bag_idx);
        start_slot_idx = ini->GetLongValue(Name(), VAR_NAME(start_slot_idx), start_slot_idx);
    }

    void SaveSettings(CSimpleIni *ini) override
    {
        HelperBoxWindow::SaveSettings(ini);
        ini->SetLongValue(Name(), VAR_NAME(bag_idx), bag_idx);
        ini->SetLongValue(Name(), VAR_NAME(start_slot_idx), start_slot_idx);
    }

    void DrawSettingInternal() override
    {
        static int _bag_idx = bag_idx;
        static int _start_slot_idx = start_slot_idx;
        const auto width = ImGui::GetWindowWidth();
        ImGui::Text("Low HP Armor Slots:");

        ImGui::Text("Bag Idx (starts at 1):");
        ImGui::SameLine(width * 0.5F);
        ImGui::PushItemWidth(width * 0.5F);
        ImGui::InputInt("###inputBagIdx", &_bag_idx, 1, 1);
        ImGui::PopItemWidth();
        bag_idx = _bag_idx;

        ImGui::Text("First Armor Piece Idx (starts at 1):");
        ImGui::SameLine(width * 0.5F);
        ImGui::PushItemWidth(width * 0.5F);
        ImGui::InputInt("###inputStartSlot", &_start_slot_idx, 1, 1);
        ImGui::PopItemWidth();
        start_slot_idx = _start_slot_idx;
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

    uint32_t bag_idx = static_cast<uint32_t>(-1);
    uint32_t start_slot_idx = static_cast<uint32_t>(-1);
    uint32_t move_idx = 0;
    std::array<Move, 23> moves = {
        Move{+1248.00F, +6965.51F, "Spawn"},
        Move{-583.28F, +9275.68F, "Lab Stairs1"},
        Move{-2730.79F, 10159.21F, "Lab Stairs2"},
        Move{-5751.45F, 12746.52F, "Lab Reaper"},
        Move{-6263.69F, 9863.36F, "Fuse Pull 1", [&]() { return ChangeFullArmor(bag_idx, start_slot_idx); }},
        Move{-6241.24F, 7945.73F, "Basement"},
        Move{-8763.36F, 5551.18F, "Basement Stairs"},
        Move{-7814.81F, 4397.92F, "Fuse Pull 2"},
        Move{-8764.08F, 2156.60F, "Vale Entry", [&]() { return ChangeFullArmor(bag_idx, start_slot_idx); }},
        Move{-12264.12F, 1821.18F, "Vale House"},
        Move{-13872.34F, 2332.34F, "Spirits 1"},
        Move{-13760.19F, 358.15F, "Spirits 2"},
        Move{-12145.44F, 1101.74F, "Spirits 3"},
        Move{-8764.08F, 2156.60F, "Vale Entry"},
        Move{-7980.55F, 4308.90F, "Basement Stairs"},
        Move{-6241.24F, 7945.73F, "Basement"},
        Move{-5751.45F, 12746.52F, "Lab Reaper"},
        Move{-6035.29F, 11285.14F, "Keeper 1"},
        Move{-3793.78F, 11200.36F, "Keeper 2"},
        Move{-3881.71F, 11280.04F, "Keeper 3"},
        Move{-1502.45F, 9737.64F, "Keeper 4/5"},
        Move{-266.03F, 9304.26F, "Lab Stairs1"},
        Move{1207.05F, 7732.16F, "Keeper 6"}};

    GW::HookEntry MapLoaded_Entry;
    bool load_cb_triggered = false;

    FuseRange fuse_pull;
    Pumping pumping;
    TankBonding tank_bonding;
    PlayerBonding player_bonding;
};

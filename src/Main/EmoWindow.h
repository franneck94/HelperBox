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
#include <Types.h>
#include <UwHelper.h>

#include <Base/HelperBoxWindow.h>

class Pumping : public EmoActionABC
{
public:
    Pumping(Player *p, EmoSkillbar *s, uint32_t *_bag_idx, uint32_t *_start_slot_idx);

    RoutineState Routine() override;
    void Update() override;

private:
    bool PauseRoutine();
    bool ResumeRoutine();

    RoutineState RoutineSelfBonds() const;
    RoutineState RoutineCanthaGuards() const;
    RoutineState RoutineLT() const;
    RoutineState RoutineDbBeforeDhuum() const;
    RoutineState RoutineTurtle() const;
    RoutineState RoutineDbAtDhuum() const;
    RoutineState RoutineWisdom() const;
    RoutineState RoutineGDW() const;
    RoutineState RoutineTurtleGDW() const;
    RoutineState RoutinePI(const uint32_t dhuum_id) const;
    RoutineState RoutineKeepPlayerAlive() const;

    static auto constexpr cantha_ids = std::array<uint32_t, 4>{8990U, 8991U, 8992U, 8993U};

    uint32_t *bag_idx = nullptr;
    uint32_t *start_slot_idx = nullptr;

    GW::HookEntry Summon_AgentAdd_Entry;
    bool found_turtle = false;
    uint32_t turtle_id = 0;

    GW::Agent *lt_agent = nullptr;
    GW::Agent *db_agent = nullptr;

    std::vector<PlayerMapping> party_members{};
    bool party_data_valid = false;
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
                if (action_state == ActionState::ACTIVE && player->SkillStoppedCallback(packet))
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
                if (action_state == ActionState::ACTIVE && player->SkillStoppedCallback(packet))
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
        static auto _bag_idx = static_cast<int>(bag_idx);
        static auto _start_slot_idx = static_cast<int>(start_slot_idx);

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
    void UpdateUw();
    void UpdateUwEntry();
    void UpdateUwMoves();

    bool ActivationConditions() const;
    void WarnDistanceLT() const;


    Player player;
    EmoSkillbar skillbar;

    uint32_t bag_idx = static_cast<uint32_t>(-1);
    uint32_t start_slot_idx = static_cast<uint32_t>(-1);

    uint32_t move_idx = 0;
    std::array<Move, 38> moves = {
        Move{1248.00F, 6965.51F, "Spawn", MoveState::NONE},
        Move{-634.07F, 9071.42F, "Lab Stairs1", MoveState::NONE},
        Move{-2126.06F, 10601.70F, "Lab Stairs2", MoveState::NONE},
        Move{-6263.33F, 9899.79F, "Fuse Pull1", MoveState::NONE, [&]() { ToLowArmor(bag_idx, start_slot_idx); }},
        Move{-5399.54F, 9022.45F, "Basement Stairs 1", MoveState::NONE},
        Move{-6241.24F, 7945.73F, "Basement", MoveState::NONE},
        Move{-8763.36F, 5551.18F, "Basement Stairs 2", MoveState::NONE},
        Move{-7829.98F, 4324.09F, "Fuse Pull2", MoveState::NONE},
        Move{-8764.08F, 2156.60F, "Bridge", MoveState::NONE, [&]() { ToHighArmor(bag_idx, start_slot_idx); }},
        Move{-8764.08F, 2156.60F, "Vale Entry", MoveState::NONE},
        Move{-12264.12F, 1821.18F, "Vale House", MoveState::NONE},
        Move{-12145.44F, 1101.74F, "Vale Center", MoveState::NONE},
        Move{-13872.34F, 2332.34F, "Spirits 1", MoveState::NONE},
        Move{-13760.19F, 358.15F, "Spirits 2", MoveState::NONE},
        Move{-12145.44F, 1101.74F, "Vale Center", MoveState::DONT_WAIT},
        Move{-8764.08F, 2156.60F, "Vale Entry", MoveState::DONT_WAIT},
        Move{-7980.55F, 4308.90F, "Basement Stairs", MoveState::DONT_WAIT},
        Move{-6241.24F, 7945.73F, "Basement", MoveState::DONT_WAIT},
        Move{-5751.45F, 12746.52F, "Lab Reaper", MoveState::NONE, [&]() { TargetClosestReaper(player); }},
        Move{-6035.29F, 11285.14F, "Keeper 1", MoveState::NONE, [&]() { TargetClosestKeeper(player); }},
        Move{-6511.41F, 12447.65F, "Keeper 2", MoveState::NONE, [&]() { TargetClosestKeeper(player); }},
        Move{-3881.71F, 11280.04F, "Keeper 3", MoveState::NONE, [&]() { TargetClosestKeeper(player); }},
        Move{-1502.45F, 9737.64F, "Keeper 4/5", MoveState::NONE, [&]() { TargetClosestKeeper(player); }},
        Move{-266.03F, 9304.26F, "Lab Stairs1", MoveState::DONT_WAIT},
        Move{1207.05F, 7732.16F, "Keeper 6", MoveState::NONE},
        Move{819.44F, 9769.97F, "To Wastes 1", MoveState::DONT_WAIT},
        Move{2247.60F, 10529.446F, "To Wastes 2", MoveState::DONT_WAIT},
        Move{3247.06F, 9099.98F, "To Wastes 3", MoveState::DONT_WAIT},
        Move{3853.85F, 7802.04F, "To Wastes 4", MoveState::DONT_WAIT},
        Move{5498.42F, 8995.82F, "To Wastes 5", MoveState::DONT_WAIT},
        Move{8389.26F, 12009.86F, "To Wastes 6", MoveState::NONE},
        Move{6633.37F, 15385.31F, "Wastes 1", MoveState::DONT_WAIT},
        Move{4337.96F, 16187.32F, "Wastes 2", MoveState::DONT_WAIT},
        Move{2448.44F, 17789.23F, "Wastes 3", MoveState::NONE},
        Move{-2537.51F, 19139.91F, "To Dhuum 1", MoveState::DONT_WAIT},
        Move{-6202.59F, 18704.91F, "To Dhuum 2", MoveState::DONT_WAIT},
        Move{-9567.56F, 17288.916F, "To Dhuum 3", MoveState::DONT_WAIT},
        Move{-13127.69F, 17284.64F, "To Dhuum 4", MoveState::NONE},
    };

    GW::HookEntry MapLoaded_Entry;
    bool load_cb_triggered = false;

    FuseRange fuse_pull;
    Pumping pumping;
    TankBonding tank_bonding;
    PlayerBonding player_bonding;
};

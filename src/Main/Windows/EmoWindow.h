#pragma once

#include "stdafx.h"

#include <array>
#include <cstdint>

#include <GWCA/Packets/StoC.h>
#include <GWCA/Utilities/Hook.h>

#include <Actions.h>
#include <Callbacks.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <Move.h>
#include <PlayerData.h>
#include <Types.h>
#include <UwHelper.h>

#include <Base/HelperBoxWindow.h>

class Pumping : public EmoActionABC
{
public:
    Pumping(PlayerData *p, EmoSkillbarData *s, uint32_t *_bag_idx, uint32_t *_slot_idx);

    RoutineState Routine() override;
    void Update() override;

private:
    bool PauseRoutine() override;

    bool RoutineWhenInRangeBondLT() const;
    bool RoutineSelfBonds() const;
    bool RoutineCanthaGuards() const;
    bool RoutineLT() const;
    bool DropBondsLT() const;
    bool RoutineDbBeforeDhuum() const;
    bool RoutineTurtle() const;
    bool RoutineDbAtDhuum() const;
    bool RoutineWisdom() const;
    bool RoutineGDW() const;
    bool RoutineTurtleGDW() const;
    bool RoutineKeepPlayerAlive() const;

    uint32_t *bag_idx = nullptr;
    uint32_t *slot_idx = nullptr;

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
    TankBonding(PlayerData *p, EmoSkillbarData *s) : EmoActionABC(p, "Tank Bonds", s)
    {
        GW::StoC::RegisterPacketCallback<GW::Packet::StoC::GenericValue>(
            &GenericValue_Entry,
            [this](GW::HookStatus *, GW::Packet::StoC::GenericValue *packet) -> void {
                if (action_state == ActionState::ACTIVE && player_data->SkillStoppedCallback(packet))
                    interrupted = true;
            });
    }

    RoutineState Routine() override;
    void Update() override;

    bool interrupted = false;
    GW::HookEntry GenericValue_Entry;
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
        first_frame = true;
    }

    void LoadSettings(CSimpleIni *ini) override
    {
        HelperBoxWindow::LoadSettings(ini);
        show_debug_map = ini->GetBoolValue(Name(), VAR_NAME(show_debug_map), show_debug_map);
        bag_idx = ini->GetLongValue(Name(), VAR_NAME(bag_idx), bag_idx);
        slot_idx = ini->GetLongValue(Name(), VAR_NAME(slot_idx), slot_idx);
    }

    void SaveSettings(CSimpleIni *ini) override
    {
        HelperBoxWindow::SaveSettings(ini);
        ini->SetBoolValue(Name(), VAR_NAME(show_debug_map), show_debug_map);
        ini->SetLongValue(Name(), VAR_NAME(bag_idx), bag_idx);
        ini->SetLongValue(Name(), VAR_NAME(slot_idx), slot_idx);
    }

    void DrawSettingInternal() override
    {
        static auto _bag_idx = static_cast<int>(bag_idx);
        static auto _slot_idx = static_cast<int>(slot_idx);

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
        ImGui::InputInt("###inputStartSlot", &_slot_idx, 1, 1);
        ImGui::PopItemWidth();
        slot_idx = _slot_idx;

        ImGui::Text("Show Debug Map:");
        ImGui::SameLine(width * 0.5F);
        ImGui::PushItemWidth(width * 0.5F);
        ImGui::Checkbox("debugMapActive", &show_debug_map);
        ImGui::PopItemWidth();
    }

    void Draw(IDirect3DDevice9 *pDevice) override;
    void Update(float delta, const PlayerData &, const AgentLivingData &) override;

private:
    void UpdateUw();
    void UpdateUwEntry();
    void UpdateUwDetectKeeper();

    void WarnDistanceLT() const;

    PlayerData player_data;
    bool first_frame = false;
    EmoSkillbarData skillbar;

    // Settings
    bool show_debug_map = true;
    uint32_t bag_idx = static_cast<uint32_t>(-1);
    uint32_t slot_idx = static_cast<uint32_t>(-1);

    std::function<bool()> swap_to_high_armor_fn = [&]() { return HighArmor(bag_idx, slot_idx); };
    std::function<bool()> swap_to_low_armor_fn = [&]() { return LowArmor(bag_idx, slot_idx); };
    std::function<bool()> target_reaper_fn = [&]() { return TargetReaper(player_data); };
    std::function<bool()> talk_reaper_fn = [&]() { return TalkReaper(player_data); };
    std::function<bool()> take_uwg_fn = [&]() { return TakeUWG(); };

    uint32_t move_idx = 0;
    std::array<Move, 55> moves = {
        Move{1248.00F, 6965.51F, "Spawn", MoveState::WAIT_AND_CONTINUE},
        Move{985.70F, 7325.54F, "Chamber 1", MoveState::WAIT_AND_CONTINUE, swap_to_high_armor_fn},
        Move{-634.07F, 9071.42F, "Chamber 2", MoveState::WAIT_AND_CONTINUE},
        Move{-1522.58F, 10634.12F, "Lab 1", MoveState::WAIT_AND_CONTINUE},
        Move{-2726.856F, 10239.48F, "Lab 2", MoveState::WAIT_AND_CONTINUE},
        Move{-2828.35F, 10020.46F, "Lab 3", MoveState::WAIT_AND_CONTINUE},
        Move{-4012.72F, 11130.53F, "Lab 4", MoveState::WAIT_AND_CONTINUE},
        Move{-4390.63F, 11647.44F, "Lab 5", MoveState::WAIT_AND_CONTINUE},
        Move{-5751.45F, 12746.52F, "Lab Reaper", MoveState::WAIT_AND_CONTINUE, target_reaper_fn},
        Move{-6263.33F, 9899.79F, "Fuse 1", MoveState::DISTANCE_AND_CONTINUE, swap_to_low_armor_fn, 3700.0F},
        Move{-5183.64F, 8876.31F, "Basement Stairs 1", MoveState::DISTANCE_AND_CONTINUE, 2000.0F},
        Move{-6241.24F, 7945.73F, "Basement Mid", MoveState::DISTANCE_AND_CONTINUE, 2000.0F},
        Move{-8798.22F, 5643.86F, "Basement Stairs 2", MoveState::DISTANCE_AND_CONTINUE, 4000.0F},
        Move{-7887.61F, 4279.11F, "Fuse 2", MoveState::DISTANCE_AND_CONTINUE, 3400.0F},
        Move{-7289.94F, 3283.81F, "Vale Door", MoveState::DISTANCE_AND_CONTINUE, swap_to_high_armor_fn, 3400.0F},
        Move{-7846.65F, 2234.26F, "Vale Bridge", MoveState::DISTANCE_AND_CONTINUE, 3400.0F},
        Move{-8764.08F, 2156.60F, "Vale Entry", MoveState::WAIT_AND_CONTINUE},
        Move{-12264.12F, 1821.18F, "Vale House", MoveState::DISTANCE_AND_CONTINUE, 3400.0F},
        Move{-12145.44F, 1101.74F, "Vale Center", MoveState::DISTANCE_AND_CONTINUE, 3200.0F},
        Move{-13872.34F, 2332.34F, "Spirits 1", MoveState::NO_WAIT_AND_CONTINUE},
        Move{-13760.19F, 358.15F, "Spirits 2", MoveState::WAIT_AND_CONTINUE},
        Move{-12145.44F, 1101.74F, "Vale Center", MoveState::WAIT_AND_CONTINUE},
        Move{-8764.08F, 2156.60F, "Vale Entry", MoveState::NO_WAIT_AND_CONTINUE},
        Move{-7980.55F, 4308.90F, "Basement Stairs", MoveState::NO_WAIT_AND_CONTINUE},
        Move{-6241.24F, 7945.73F, "Basement", MoveState::WAIT_AND_CONTINUE},
        Move{-5751.45F, 12746.52F, "Lab Reaper", MoveState::NO_WAIT_AND_STOP, target_reaper_fn},
        Move{-5751.45F, 12746.52F, "Talk", MoveState::NO_WAIT_AND_CONTINUE, talk_reaper_fn},
        Move{-5751.45F, 12746.52F, "UWG", MoveState::NO_WAIT_AND_CONTINUE, take_uwg_fn},
        Move{-6035.29F, 11285.14F, "Keeper 1", MoveState::NO_WAIT_AND_STOP},
        Move{-6511.41F, 12447.65F, "Keeper 2", MoveState::NO_WAIT_AND_STOP},
        Move{-3881.71F, 11280.04F, "Keeper 3", MoveState::NO_WAIT_AND_STOP},
        Move{-1502.45F, 9737.64F, "Keeper 4/5", MoveState::NO_WAIT_AND_STOP},
        Move{-266.03F, 9304.26F, "Lab 1", MoveState::NO_WAIT_AND_CONTINUE},
        Move{1207.05F, 7732.16F, "Keeper 6", MoveState::NO_WAIT_AND_STOP},
        Move{819.44F, 9769.97F, "To Wastes 1", MoveState::NO_WAIT_AND_CONTINUE},
        Move{2247.60F, 10529.446F, "To Wastes 2", MoveState::NO_WAIT_AND_CONTINUE},
        Move{3247.06F, 9099.98F, "To Wastes 3", MoveState::NO_WAIT_AND_CONTINUE},
        Move{3853.85F, 7802.04F, "To Wastes 4", MoveState::NO_WAIT_AND_CONTINUE},
        Move{5498.42F, 8995.82F, "To Wastes 5", MoveState::NO_WAIT_AND_CONTINUE},
        Move{6831.40F, 11142.24F, "To Wastes 6", MoveState::NO_WAIT_AND_STOP},
        Move{6633.37F, 15385.31F, "Wastes 1", MoveState::NO_WAIT_AND_STOP},
        Move{6054.83F, 18997.46F, "Wastes 2", MoveState::NO_WAIT_AND_CONTINUE},
        Move{4968.64F, 16555.77F, "Wastes 3", MoveState::WAIT_AND_CONTINUE},
        Move{2152.55F, 16893.93F, "Wastes 4", MoveState::NO_WAIT_AND_STOP},
        Move{8685.21F, 6344.59F, "Pits Start", MoveState::NO_WAIT_AND_CONTINUE},
        Move{12566.49F, 7812.503F, "Pits 1", MoveState::NO_WAIT_AND_CONTINUE},
        Move{11958.36F, 6281.43F, "Pits 2", MoveState::NO_WAIT_AND_STOP},
        Move{11368.55F, -17974.64F, "Planes Start", MoveState::NO_WAIT_AND_CONTINUE},
        Move{12160.99F, -16830.55F, "Planes 1", MoveState::NO_WAIT_AND_STOP},
        Move{-235.05F, 18496.461F, "To Dhuum Start", MoveState::NO_WAIT_AND_CONTINUE},
        Move{-2537.51F, 19139.91F, "To Dhuum 1", MoveState::NO_WAIT_AND_CONTINUE},
        Move{-6202.59F, 18704.91F, "To Dhuum 2", MoveState::NO_WAIT_AND_CONTINUE},
        Move{-9567.56F, 17288.916F, "To Dhuum 3", MoveState::NO_WAIT_AND_CONTINUE, swap_to_low_armor_fn},
        Move{-13127.69F, 17284.64F, "To Dhuum 4", MoveState::NO_WAIT_AND_CONTINUE},
        Move{-16105.50F, 17284.84F, "To Dhuum 5", MoveState::NO_WAIT_AND_STOP},
    };

    GW::HookEntry MapLoaded_Entry;
    bool load_cb_triggered = false;

    Pumping pumping;
    TankBonding tank_bonding;
};
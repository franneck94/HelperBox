#pragma once

#include <array>
#include <cstdint>

#include <GWCA/GameEntities/Agent.h>
#include <GWCA/Packets/StoC.h>
#include <GWCA/Utilities/Hook.h>

#include <Actions.h>
#include <AgentData.h>
#include <Base/HelperBoxWindow.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <HelperCallbacks.h>
#include <HelperItems.h>
#include <HelperUw.h>
#include <Move.h>
#include <PlayerData.h>
#include <Types.h>

#include <SimpleIni.h>
#include <imgui.h>

class Pumping : public EmoActionABC
{
public:
    Pumping(PlayerData *p, EmoSkillbarData *s, uint32_t *_bag_idx, uint32_t *_slot_idx, const AgentLivingData *a);

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

    const uint32_t *bag_idx = nullptr;
    const uint32_t *slot_idx = nullptr;

    GW::HookEntry Summon_AgentAdd_Entry;
    bool found_turtle = false;
    uint32_t turtle_id = 0;

    const GW::Agent *lt_agent = nullptr;
    const GW::Agent *db_agent = nullptr;

    std::vector<PlayerMapping> party_members{};
    bool party_data_valid = false;

    const AgentLivingData *agents_data = nullptr;
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
    void Update(float delta, const AgentLivingData &) override;

private:
    void UpdateUw();
    void UpdateUwEntry();

    PlayerData player_data;
    const AgentLivingData *agents_data = nullptr;
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
    std::array<MoveABC *, 56> moves = {
        new Move_WaitAndContinue{1248.00F, 6965.51F, "Spawn"},
        new Move_WaitAndContinue{985.70F, 7325.54F, "Chamber 1", swap_to_high_armor_fn},
        new Move_WaitAndContinue{-634.07F, 9071.42F, "Chamber 2"},
        new Move_WaitAndContinue{-1522.58F, 10634.12F, "Lab 1"},
        new Move_WaitAndContinue{-2726.856F, 10239.48F, "Lab 2"},
        new Move_WaitAndContinue{-2828.35F, 10020.46F, "Lab 3"},
        new Move_WaitAndContinue{-4012.72F, 11130.53F, "Lab 4"},
        new Move_WaitAndContinue{-4390.63F, 11647.44F, "Lab 5"},
        new Move_WaitAndContinue{-5751.45F, 12746.52F, "Lab Reaper", target_reaper_fn},
        new Move_DistanceAndContinue{-5830.34F, 11781.70F, "To Fuse 1", 3700.0F},
        new Move_NoWaitAndContinue{-6263.33F, 9899.79F, "Fuse 1", swap_to_low_armor_fn},
        new Move_DistanceAndContinue{-5183.64F, 8876.31F, "Basement Stairs 1", 2000.0F},
        new Move_DistanceAndContinue{-6241.24F, 7945.73F, "Basement Mid", 2000.0F},
        new Move_DistanceAndContinue{-8798.22F, 5643.86F, "Basement Stairs 2", 4000.0F},
        new Move_DistanceAndContinue{-7887.61F, 4279.11F, "Fuse 2", 3400.0F},
        new Move_DistanceAndContinue{-7289.94F, 3283.81F, "Vale Door", 3400.0F, swap_to_high_armor_fn},
        new Move_DistanceAndContinue{-7846.65F, 2234.26F, "Vale Bridge", 3400.0F},
        new Move_WaitAndContinue{-8764.08F, 2156.60F, "Vale Entry"},
        new Move_DistanceAndContinue{-12264.12F, 1821.18F, "Vale House", 3400.0F},
        new Move_DistanceAndContinue{-12145.44F, 1101.74F, "Vale Center", 3200.0F},
        new Move_NoWaitAndContinue{-13872.34F, 2332.34F, "Spirits 1"},
        new Move_WaitAndContinue{-13760.19F, 358.15F, "Spirits 2"},
        new Move_WaitAndContinue{-12145.44F, 1101.74F, "Vale Center"},
        new Move_NoWaitAndContinue{-8764.08F, 2156.60F, "Vale Entry"},
        new Move_NoWaitAndContinue{-7980.55F, 4308.90F, "Basement Stairs"},
        new Move_WaitAndContinue{-6241.24F, 7945.73F, "Basement"},
        new Move_NoWaitAndStop{-5751.45F, 12746.52F, "Lab Reaper", target_reaper_fn},
        new Move_NoWaitAndContinue{-5751.45F, 12746.52F, "Talk", talk_reaper_fn},
        new Move_NoWaitAndContinue{-5751.45F, 12746.52F, "UWG", take_uwg_fn},
        new Move_NoWaitAndContinue{-6035.29F, 11285.14F, "Keeper 1"},
        new Move_WaitAndStop{-6511.41F, 12447.65F, "Keeper 2"},
        new Move_NoWaitAndStop{-3881.71F, 11280.04F, "Keeper 3"},
        new Move_DistanceAndContinue{-1502.45F, 9737.64F, "Keeper 4/5", 4300.0F},
        new Move_DistanceAndContinue{-266.03F, 9304.26F, "Lab 1", 4300.0F},
        new Move_NoWaitAndStop{1207.05F, 7732.16F, "Keeper 6"},
        new Move_NoWaitAndContinue{819.44F, 9769.97F, "To Wastes 1"},
        new Move_NoWaitAndContinue{2247.60F, 10529.446F, "To Wastes 2"},
        new Move_NoWaitAndContinue{3247.06F, 9099.98F, "To Wastes 3"},
        new Move_NoWaitAndContinue{3853.85F, 7802.04F, "To Wastes 4"},
        new Move_NoWaitAndContinue{5498.42F, 8995.82F, "To Wastes 5"},
        new Move_NoWaitAndStop{6831.40F, 11142.24F, "To Wastes 6"},
        new Move_NoWaitAndStop{6633.37F, 15385.31F, "Wastes 1"},
        new Move_NoWaitAndContinue{6054.83F, 18997.46F, "Wastes 2"},
        new Move_WaitAndContinue{4968.64F, 16555.77F, "Wastes 3"},
        new Move_NoWaitAndStop{2152.55F, 16893.93F, "Wastes 4"},
        new Move_NoWaitAndContinue{8685.21F, 6344.59F, "Pits Start"},
        new Move_NoWaitAndContinue{12566.49F, 7812.503F, "Pits 1"},
        new Move_NoWaitAndStop{11958.36F, 6281.43F, "Pits 2"},
        new Move_NoWaitAndContinue{11368.55F, -17974.64F, "Planes Start"},
        new Move_WaitAndStop{12160.99F, -16830.55F, "Planes 1"},
        new Move_NoWaitAndContinue{-235.05F, 18496.461F, "To Dhuum Start"},
        new Move_NoWaitAndContinue{-2537.51F, 19139.91F, "To Dhuum 1"},
        new Move_NoWaitAndContinue{-6202.59F, 18704.91F, "To Dhuum 2"},
        new Move_NoWaitAndContinue{-9567.56F, 17288.916F, "To Dhuum 3", swap_to_low_armor_fn},
        new Move_NoWaitAndContinue{-13127.69F, 17284.64F, "To Dhuum 4"},
        new Move_NoWaitAndStop{-16105.50F, 17284.84F, "To Dhuum 5"},
    };

    GW::HookEntry MapLoaded_Entry;
    bool load_cb_triggered = false;

    Pumping pumping;
    TankBonding tank_bonding;
};

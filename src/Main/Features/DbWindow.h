#pragma once

#include <array>
#include <cstdint>

#include <GWCA/Packets/StoC.h>
#include <GWCA/Utilities/Hook.h>

#include <Base/HelperBoxWindow.h>

#include <Actions.h>
#include <AgentData.h>
#include <GuiUtils.h>
#include <HelperCallbacks.h>
#include <HelperUw.h>
#include <Move.h>
#include <PlayerData.h>
#include <Types.h>

#include <SimpleIni.h>
#include <imgui.h>

class Damage : public DbActionABC
{
public:
    Damage(PlayerData *p, DbSkillbarData *s, const AgentLivingData *a);

    RoutineState Routine() override;
    void Update() override;

private:
    bool PauseRoutine() override;

    bool CastPiOnTarget() const;
    bool RoutineKillSkele() const;
    bool RoutineValeSpirits() const;
    bool RoutineDhuumRecharge() const;
    bool RoutineDhuumDamage() const;

    const AgentLivingData *agents_data = nullptr;
};

class DbWindow : public HelperBoxWindow
{
public:
    DbWindow();
    ~DbWindow(){};

    static DbWindow &Instance()
    {
        static DbWindow instance;
        return instance;
    }

    const char *Name() const override
    {
        return "DbWindow";
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
    }

    void SaveSettings(CSimpleIni *ini) override
    {
        HelperBoxWindow::SaveSettings(ini);
        ini->SetBoolValue(Name(), VAR_NAME(show_debug_map), show_debug_map);
    }

    void DrawSettingInternal() override
    {
#ifdef _DEBUG
        const auto width = ImGui::GetWindowWidth();
        ImGui::Text("Show Debug Map:");
        ImGui::SameLine(width * 0.5F);
        ImGui::PushItemWidth(width * 0.5F);
        ImGui::Checkbox("debugMapActive", &show_debug_map);
        ImGui::PopItemWidth();
#endif
    }

    void Draw(IDirect3DDevice9 *pDevice) override;
    void Update(float delta, const AgentLivingData &) override;

private:
    void UpdateUw();
    void UpdateUwEntry();

    // Settings
    bool show_debug_map = true;

    PlayerData player_data;
    const AgentLivingData *agents_data = nullptr;
    bool first_frame = false;
    DbSkillbarData skillbar;

    std::function<bool()> target_reaper_fn = [&]() { return TargetReaper(player_data); };
    std::function<bool()> talk_reaper_fn = [&]() { return TalkReaper(player_data); };
    std::function<bool()> cast_sq = [&]() {
        skillbar.sq.Cast(player_data.energy);
        return true;
    };

    uint32_t move_idx = 0;
    std::array<MoveABC *, 62> moves = {
        new Move_WaitAndStop{1248.00F, 6965.51F, "Spawn"},
        new Move_WaitAndContinue{613.38F, 7097.03F, "SQ", cast_sq},
        new Move_WaitAndContinue{314.57F, 7511.54F, "MoveABC EoE 1"},
        new Move_CastSkillAndContinue{314.57F, 7511.54F, "EoE 1", &skillbar.eoe},
        new Move_WaitAndContinue{314.57F, 7511.54F, "MoveABC EoE 1"},
        new Move_WaitAndContinue{1319.41F, 7299.941F, "MoveABC Qz"},
        new Move_CastSkillAndContinue{1319.41F, 7299.94F, "Qz", &skillbar.qz},
        new Move_WaitAndContinue{1319.41F, 7299.941F, "MoveABC Qz"},
        new Move_WaitAndContinue{985.70F, 7325.54F, "Chamber 1"},
        new Move_WaitAndContinue{-634.07F, 9071.42F, "Chamber 2"},
        new Move_WaitAndContinue{-1522.58F, 10634.12F, "Lab 1"},
        new Move_WaitAndContinue{-2726.856F, 10239.48F, "Lab 2"},
        new Move_WaitAndContinue{-2828.35F, 10020.46F, "Lab 3"},
        new Move_WaitAndContinue{-4012.72F, 11130.53F, "Lab 4", cast_sq},
        new Move_CastSkillAndContinue{-4012.72F, 11130.53F, "EoE 2", &skillbar.eoe},
        new Move_WaitAndContinue{-4012.72F, 11130.53F, "Lab 4"},
        new Move_WaitAndContinue{-4390.63F, 11647.44F, "Lab 5"},
        new Move_WaitAndStop{-5751.45F, 12746.52F, "Lab Reaper", target_reaper_fn},
        new Move_NoWaitAndContinue{-5751.45F, 12746.52F, "Talk", talk_reaper_fn},
        new Move_NoWaitAndContinue{-5751.45F, 12746.52F, "Accept", [&]() { return AcceptChamber(); }},
        new Move_NoWaitAndContinue{-5751.45F, 12746.52F, "Restore", [&]() { return TakeRestore(); }},
        new Move_WaitAndContinue{-5751.45F, 12746.52F, "Escort", [&]() { return TakeEscort(); }},
        new Move_WaitAndContinue{-6622.24F, 10387.12F, "Basement Stairs", cast_sq},
        new Move_CastSkillAndContinue{-6622.24F, 10387.12F, "EoE 3", &skillbar.eoe},
        new Move_WaitAndContinue{-6622.24F, 10387.12F, "Basement Stairs"},
        new Move_DistanceAndContinue{-5183.64F, 8876.31F, "Basement Stairs 1", 2200.0F},
        new Move_DistanceAndContinue{-6241.24F, 7945.73F, "Basement Mid", 2000.0F},
        new Move_DistanceAndContinue{-8798.22F, 5643.86F, "Basement Stairs 2", 4000.0F},
        new Move_WaitAndContinue{-8518.53F, 4765.09F, "Basement 3", cast_sq},
        new Move_CastSkillAndContinue{-8518.53F, 4765.09F, "EoE 4", &skillbar.eoe},
        new Move_WaitAndContinue{-8518.53F, 4765.09F, "Basement 3"},
        new Move_DistanceAndContinue{-7289.94F, 3283.81F, "Vale Door", 3400.0F},
        new Move_DistanceAndContinue{-7846.65F, 2234.26F, "Vale Bridge", 3400.0F},
        new Move_WaitAndContinue{-8764.08F, 2156.60F, "Vale Entry"},
        new Move_DistanceAndContinue{-12264.12F, 1821.18F, "Vale House", 3400.0F},
        new Move_CastSkillAndContinue{-12264.12F, 1821.18F, "EoE 5", &skillbar.eoe},
        new Move_DistanceAndContinue{-12264.12F, 1821.18F, "Vale House", 3500.0F},
        new Move_DistanceAndContinue{-12145.44F, 1101.74F, "Vale Center", 3200.0F},
        new Move_NoWaitAndContinue{-13760.19F, 358.15F, "Spirits 1"},
        new Move_DistanceAndContinue{-13872.34F, 2332.34F, "Spirits 2", 4900.0F},
        new Move_WaitAndStop{-13312.71F, 5165.071F, "Vale Reaper", target_reaper_fn},
        new Move_NoWaitAndContinue{8685.21F, 6344.59F, "Pits Start"},
        new Move_NoWaitAndContinue{12566.49F, 7812.503F, "Pits 1"},
        new Move_CastSkillAndContinue{12566.49F, 7812.503F, "Pits Winnow", &skillbar.winnow},
        new Move_NoWaitAndContinue{12566.49F, 7812.503F, "Pits 1"},
        new Move_NoWaitAndStop{8685.21F, 6344.59F, "Pits Reaper", target_reaper_fn},
        new Move_NoWaitAndContinue{8685.21F, 6344.59F, "Pits Talk", talk_reaper_fn},
        new Move_NoWaitAndStop{8685.21F, 6344.59F, "Pits Take", [&]() { return TakePits(); }},
        new Move_NoWaitAndContinue{11368.55F, -17974.64F, "Planes Start"},
        new Move_NoWaitAndContinue{9120.00F, -18432.003F, "Planes 1"},
        new Move_CastSkillAndContinue{9120.00F, -18432.003F, "Planes Winnow", &skillbar.winnow},
        new Move_CastSkillAndContinue{9120.00F, -18432.003F, "Planes EoE", &skillbar.eoe},
        new Move_NoWaitAndContinue{9120.00F, -18432.003F, "Planes 1"},
        new Move_NoWaitAndStop{11368.55F, -17974.64F, "Planes Reaper", target_reaper_fn},
        new Move_NoWaitAndContinue{11368.55F, -17974.64F, "Planes Talk", talk_reaper_fn},
        new Move_NoWaitAndStop{11368.55F, -17974.64F, "Planes Take", [&]() { return TakePlanes(); }},
        new Move_NoWaitAndContinue{-235.05F, 18496.461F, "To Dhuum Start"},
        new Move_NoWaitAndContinue{-2537.51F, 19139.91F, "To Dhuum 1"},
        new Move_NoWaitAndContinue{-6202.59F, 18704.91F, "To Dhuum 2"},
        new Move_NoWaitAndContinue{-9567.56F, 17288.916F, "To Dhuum 3"},
        new Move_NoWaitAndContinue{-13127.69F, 17284.64F, "To Dhuum 4"},
        new Move_NoWaitAndStop{-16410.75F, 17294.47F, "To Dhuum 5"},
    };

    GW::HookEntry MapLoaded_Entry;
    bool load_cb_triggered = false;
    GW::HookEntry GenericValue_Entry;
    bool interrupted = false;

    Damage damage;
};

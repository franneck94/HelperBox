#pragma once

#include "stdafx.h"

#include <array>
#include <cstdint>

#include <GWCA/Packets/StoC.h>
#include <GWCA/Utilities/Hook.h>

#include <Actions.h>
#include <Callbacks.h>
#include <GuiUtils.h>
#include <Move.h>
#include <Player.h>
#include <Types.h>
#include <UwHelper.h>

#include <Base/HelperBoxWindow.h>

class Damage : public DbActionABC
{
public:
    Damage(Player *p, DbSkillbar *s);

    RoutineState Routine() override;
    void Update() override;

private:
    bool PauseRoutine();
    bool ResumeRoutine();

    bool CastPiOnTarget() const;
    bool RoutineKillSkele() const;
    bool RoutineValeSpirits() const;
    bool RoutinePI(const uint32_t dhuum_id) const;
    bool RoutineDhuumRecharge() const;
    bool RoutineDhuumDamage() const;
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
        show_menubutton = true;
        show_debug_map = ini->GetBoolValue(Name(), VAR_NAME(show_debug_map), show_debug_map);
    }

    void SaveSettings(CSimpleIni *ini) override
    {
        HelperBoxWindow::SaveSettings(ini);
        ini->SetBoolValue(Name(), VAR_NAME(show_debug_map), show_debug_map);
    }

    void DrawSettingInternal() override
    {
        const auto width = ImGui::GetWindowWidth();
        ImGui::Text("Show Debug Map:");
        ImGui::SameLine(width * 0.5F);
        ImGui::PushItemWidth(width * 0.5F);
        ImGui::Checkbox("debugMapActive", &show_debug_map);
        ImGui::PopItemWidth();
    }

    void Draw(IDirect3DDevice9 *pDevice) override;
    void Update(float delta) override;

private:
    void UpdateUw();
    void UpdateUwEntry();

    bool ActivationConditions() const;

    // Settings
    bool show_debug_map = true;

    Player player;
    bool first_frame = false;
    DbSkillbar skillbar;

    std::function<bool()> target_reaper_fn = [&]() { return TargetReaper(player); };
    std::function<bool()> talk_reaper_fn = [&]() { return TalkReaper(player); };
    std::function<bool()> cast_sq = [&]() {
        skillbar.sq.Cast(player.energy);
        return true;
    };

    uint32_t move_idx = 0;
    std::array<Move, 58> moves = {
        Move{1248.00F, 6965.51F, "Spawn", MoveState::NO_WAIT_AND_STOP},
        Move{613.38F, 7097.03F, "SQ", MoveState::WAIT_AND_CONTINUE, cast_sq},
        Move{314.57F, 7511.54F, "Move EoE 1", MoveState::WAIT_AND_CONTINUE},
        Move{314.57F, 7511.54F, "EoE 1", MoveState::CAST_SKILL_AND_CONTINUE, &skillbar.eoe},
        Move{314.57F, 7511.54F, "Move EoE 1", MoveState::WAIT_AND_CONTINUE},
        Move{1319.41F, 7299.941F, "Move Qz", MoveState::WAIT_AND_CONTINUE},
        Move{1319.41F, 7299.94F, "Qz", MoveState::CAST_SKILL_AND_CONTINUE, &skillbar.qz},
        Move{1319.41F, 7299.941F, "Move Qz", MoveState::WAIT_AND_CONTINUE},
        Move{985.70F, 7325.54F, "Chamber 1", MoveState::WAIT_AND_CONTINUE},
        Move{-634.07F, 9071.42F, "Chamber 2", MoveState::WAIT_AND_CONTINUE},
        Move{-1522.58F, 10634.12F, "Lab 1", MoveState::WAIT_AND_CONTINUE},
        Move{-2726.856F, 10239.48F, "Lab 2", MoveState::WAIT_AND_CONTINUE},
        Move{-2828.35F, 10020.46F, "Lab 3", MoveState::WAIT_AND_CONTINUE},
        Move{-4012.72F, 11130.53F, "Lab 4", MoveState::WAIT_AND_CONTINUE},
        Move{-4012.72F, 11130.53F, "EoE 2", MoveState::CAST_SKILL_AND_CONTINUE, &skillbar.eoe},
        Move{-4012.72F, 11130.53F, "Lab 4", MoveState::WAIT_AND_CONTINUE},
        Move{-4470.48F, 11581.47F, "Lab 5", MoveState::WAIT_AND_CONTINUE},
        Move{-5751.45F, 12746.52F, "Lab Reaper", MoveState::WAIT_AND_STOP, target_reaper_fn},
        Move{-5751.45F, 12746.52F, "Talk", MoveState::NO_WAIT_AND_CONTINUE, talk_reaper_fn},
        Move{-5751.45F, 12746.52F, "Accept", MoveState::NO_WAIT_AND_CONTINUE, [&]() { return AcceptChamber(); }},
        Move{-5751.45F, 12746.52F, "Restore", MoveState::NO_WAIT_AND_CONTINUE, [&]() { return TakeRestore(); }},
        Move{-5751.45F, 12746.52F, "Escort", MoveState::WAIT_AND_CONTINUE, [&]() { return TakeEscort(); }},
        Move{-6622.24F, 10387.12F, "Basement Stairs", MoveState::WAIT_AND_CONTINUE},
        Move{-6622.24F, 10387.12F, "EoE 3", MoveState::CAST_SKILL_AND_CONTINUE, &skillbar.eoe},
        Move{-6622.24F, 10387.12F, "Basement Stairs", MoveState::WAIT_AND_CONTINUE},
        Move{-5183.64F, 8876.31F, "Basement Stairs 1", MoveState::WAIT_AND_CONTINUE},
        Move{-6241.24F, 7945.73F, "Basement Mid", MoveState::DISTANCE_AND_CONTINUE},
        Move{-8798.22F, 5643.86F, "Basement Stairs 2", MoveState::WAIT_AND_CONTINUE},
        Move{-8518.53F, 4765.09F, "Basement 3", MoveState::WAIT_AND_CONTINUE},
        Move{-8518.53F, 4765.09F, "EoE 4", MoveState::CAST_SKILL_AND_CONTINUE, &skillbar.eoe},
        Move{-8518.53F, 4765.09F, "Basement 3", MoveState::WAIT_AND_CONTINUE},
        Move{-7289.94F, 3283.81F, "Vale Door", MoveState::WAIT_AND_CONTINUE},
        Move{-7846.65F, 2234.26F, "Vale Bridge", MoveState::DISTANCE_AND_CONTINUE},
        Move{-8764.08F, 2156.60F, "Vale Entry", MoveState::WAIT_AND_CONTINUE},
        Move{-12264.12F, 1821.18F, "Vale House", MoveState::WAIT_AND_CONTINUE},
        Move{-12264.12F, 1821.18F, "EoE 5", MoveState::CAST_SKILL_AND_CONTINUE, &skillbar.eoe},
        Move{-12264.12F, 1821.18F, "Vale House", MoveState::WAIT_AND_STOP},
        Move{-12145.44F, 1101.74F, "Vale Center", MoveState::WAIT_AND_CONTINUE},
        Move{-13760.19F, 358.15F, "Spirits 1", MoveState::NO_WAIT_AND_STOP},
        Move{-13312.71F, 5165.071F, "Vale Reaper", MoveState::WAIT_AND_STOP, target_reaper_fn},
        Move{12566.49F, 7812.503F, "Pits 1", MoveState::NO_WAIT_AND_CONTINUE},
        Move{12566.49F, 7812.503F, "Pits Winnow", MoveState::CAST_SKILL_AND_CONTINUE, &skillbar.winnow},
        Move{12566.49F, 7812.503F, "Pits 1", MoveState::NO_WAIT_AND_CONTINUE},
        Move{8685.21F, 6344.59F, "Pits Reaper", MoveState::NO_WAIT_AND_STOP, target_reaper_fn},
        Move{8685.21F, 6344.59F, "Pits Talk", MoveState::NO_WAIT_AND_CONTINUE, talk_reaper_fn},
        Move{8685.21F, 6344.59F, "Pits Take", MoveState::NO_WAIT_AND_STOP, [&]() { return TakePits(); }},
        Move{9120.00F, -18432.003F, "Planes 1", MoveState::NO_WAIT_AND_CONTINUE},
        Move{9120.00F, -18432.003F, "Planes Winnow", MoveState::CAST_SKILL_AND_CONTINUE, &skillbar.winnow},
        Move{9120.00F, -18432.003F, "Planes EoE", MoveState::CAST_SKILL_AND_CONTINUE, &skillbar.eoe},
        Move{9120.00F, -18432.003F, "Planes 1", MoveState::NO_WAIT_AND_CONTINUE},
        Move{11368.55F, -17974.64F, "Planes Reaper", MoveState::NO_WAIT_AND_STOP, target_reaper_fn},
        Move{11368.55F, -17974.64F, "Planes Talk", MoveState::NO_WAIT_AND_CONTINUE, talk_reaper_fn},
        Move{11368.55F, -17974.64F, "Planes Take", MoveState::NO_WAIT_AND_STOP, [&]() { return TakePlanes(); }},
        Move{-2537.51F, 19139.91F, "To Dhuum 1", MoveState::NO_WAIT_AND_CONTINUE},
        Move{-6202.59F, 18704.91F, "To Dhuum 2", MoveState::NO_WAIT_AND_CONTINUE},
        Move{-9567.56F, 17288.916F, "To Dhuum 3", MoveState::NO_WAIT_AND_CONTINUE},
        Move{-13127.69F, 17284.64F, "To Dhuum 4", MoveState::NO_WAIT_AND_STOP},
        Move{-16410.75F, 17294.47F, "To Dhuum 5", MoveState::NO_WAIT_AND_STOP},
    };

    GW::HookEntry MapLoaded_Entry;
    bool load_cb_triggered = false;
    GW::HookEntry GenericValue_Entry;
    bool interrupted = false;

    Damage damage;
};

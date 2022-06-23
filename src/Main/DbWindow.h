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
    bool RoutineAtChamberSkele() const;
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
    void UpdateUwMoves();

    bool ActivationConditions() const;

    // Settings
    bool show_debug_map = true;

    Player player;
    DbSkillbar skillbar;

    uint32_t move_idx = 0;
    std::array<Move, 56> moves = {
        Move{1248.00F, 6965.51F, "Spawn", MoveState::STOP},
        Move{613.38F, 7097.03F, "SQ", MoveState::WAIT, [&]() { skillbar.sq.Cast(player.energy); }},
        Move{157.41F, 7781.66F, "Move EoE 1", MoveState::WAIT},
        Move{157.41F, 7781.66F, "EoE 1", MoveState::CAST_SKILL, &skillbar.eoe},
        Move{157.41F, 7781.66F, "Move EoE 1", MoveState::WAIT},
        Move{1319.41F, 7299.941F, "Move Qz", MoveState::WAIT},
        Move{1319.41F, 7299.94F, "Qz", MoveState::CAST_SKILL, &skillbar.qz},
        Move{1319.41F, 7299.941F, "Move Qz", MoveState::WAIT},
        Move{985.70F, 7325.54F, "Chamber 1", MoveState::WAIT},
        Move{-634.07F, 9071.42F, "Chamber 2", MoveState::WAIT},
        Move{-1522.58F, 10634.12F, "Lab 1", MoveState::DONT_WAIT},
        Move{-2726.856F, 10239.48F, "Lab 2", MoveState::WAIT},
        Move{-2828.35F, 10020.46F, "Lab 3", MoveState::WAIT},
        Move{-4012.72F, 11130.53F, "Lab 4", MoveState::WAIT},
        Move{-4012.72F, 11130.53F, "EoE 2", MoveState::CAST_SKILL, &skillbar.eoe},
        Move{-4470.48F, 11581.47F, "Lab 5", MoveState::WAIT},
        Move{-5751.45F, 12746.52F, "Lab Reaper", MoveState::STOP, [&]() { TargetClosestReaper(player); }},
        Move{-5751.45F, 12746.52F, "Talk", MoveState::STOP, [&]() { TalkClosestReaper(player); }},
        Move{-5751.45F, 12746.52F, "Accept", MoveState::STOP, [&]() { AcceptChamber(); }},
        Move{-5751.45F, 12746.52F, "Restore", MoveState::STOP, [&]() { TakeRestore(); }},
        Move{-5751.45F, 12746.52F, "Escort", MoveState::DONT_WAIT, [&]() { TakeEscort(); }},
        Move{-6622.24F, 10387.12F, "Basement Stairs", MoveState::WAIT},
        Move{-6622.24F, 10387.12F, "EoE 3", MoveState::CAST_SKILL, &skillbar.eoe},
        Move{-6622.24F, 10387.12F, "Basement Stairs", MoveState::DONT_WAIT},
        Move{-5183.64F, 8876.31F, "Basement 1", MoveState::WAIT},
        Move{-6241.24F, 7945.73F, "Basement 2", MoveState::LT_DISTANCE},
        Move{-8402.22F, 4687.86F, "Basement 3", MoveState::WAIT},
        Move{-8402.00F, 4687.26F, "EoE 4", MoveState::CAST_SKILL, &skillbar.eoe},
        Move{-8402.22F, 4687.86F, "Basement 3", MoveState::WAIT},
        Move{-7289.94F, 3283.81F, "Vale Door", MoveState::WAIT},
        Move{-7846.65F, 2234.26F, "Vale Bridge", MoveState::WAIT},
        Move{-8764.08F, 2156.60F, "Vale Entry", MoveState::LT_DISTANCE},
        Move{-12264.12F, 1821.18F, "Vale House", MoveState::DONT_WAIT},
        Move{-12264.12F, 1821.18F, "EoE 5", MoveState::CAST_SKILL, &skillbar.eoe},
        Move{-12264.12F, 1821.18F, "Vale House", MoveState::WAIT},
        Move{-12145.44F, 1101.74F, "Vale Center", MoveState::DONT_WAIT},
        Move{-13760.19F, 358.15F, "Spirits 1", MoveState::WAIT},
        Move{-13312.71F, 5165.071F, "Vale Reaper", MoveState::STOP, [&]() { TargetClosestReaper(player); }},
        Move{12566.49F, 7812.503F, "Pits 1", MoveState::DONT_WAIT},
        Move{12566.49F, 7812.503F, "Pits Winnow", MoveState::CAST_SKILL, &skillbar.winnow},
        Move{12566.49F, 7812.503F, "Pits 1", MoveState::DONT_WAIT},
        Move{8685.21F, 6344.59F, "Pits Reaper", MoveState::STOP, [&]() { TargetClosestReaper(player); }},
        Move{-5751.45F, 12746.52F, "Pits Talk", MoveState::STOP, [&]() { TalkClosestReaper(player); }},
        Move{-5751.45F, 12746.52F, "Pits Take", MoveState::STOP, [&]() { TakePits(); }},
        Move{9120.00F, -18432.003F, "Planes 1", MoveState::DONT_WAIT},
        Move{9120.00F, -18432.003F, "Planes Winnow", MoveState::CAST_SKILL, &skillbar.winnow},
        Move{9120.00F, -18432.003F, "Planes 1", MoveState::DONT_WAIT},
        Move{11368.55F, -17974.64F, "Planes Reaper", MoveState::STOP},
        Move{8685.21F, 6344.59F, "Planes Reaper", MoveState::STOP, [&]() { TargetClosestReaper(player); }},
        Move{-5751.45F, 12746.52F, "Planes Talk", MoveState::STOP, [&]() { TalkClosestReaper(player); }},
        Move{-5751.45F, 12746.52F, "Planes Take", MoveState::STOP, [&]() { TakePlanes(); }},
        Move{-2537.51F, 19139.91F, "To Dhuum 1", MoveState::DONT_WAIT},
        Move{-6202.59F, 18704.91F, "To Dhuum 2", MoveState::DONT_WAIT},
        Move{-9567.56F, 17288.916F, "To Dhuum 3", MoveState::DONT_WAIT},
        Move{-13127.69F, 17284.64F, "To Dhuum 4", MoveState::STOP},
        Move{-16105.50F, 17284.84F, "To Dhuum 5", MoveState::STOP},
    };

    GW::HookEntry MapLoaded_Entry;
    bool load_cb_triggered = false;
    GW::HookEntry GenericValue_Entry;
    bool interrupted = false;

    Damage damage;
};

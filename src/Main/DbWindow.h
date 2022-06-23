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
    }

    void SaveSettings(CSimpleIni *ini) override
    {
        HelperBoxWindow::SaveSettings(ini);
    }

    void Draw(IDirect3DDevice9 *pDevice) override;
    void Update(float delta) override;

private:
    void UpdateUw();
    void UpdateUwEntry();
    void UpdateUwMoves();

    bool ActivationConditions() const;

    Player player;
    DbSkillbar skillbar;

    uint32_t move_idx = 0;
    std::array<Move, 39> moves = {
        Move{1248.00F, 6965.51F, "Spawn", MoveState::NONE},
        Move{613.38F, 7097.03F, "SQ", MoveState::DONT_WAIT, [&]() { skillbar.sq.Cast(player.energy); }},
        Move{157.41F, 7781.66F, "Move EoE", MoveState::DONT_WAIT},
        Move{157.41F, 7781.66F, "EoE", MoveState::CAST_SKILL, &skillbar.eoe},
        Move{1319.41F, 7299.941F, "Move Qz", MoveState::DONT_WAIT},
        Move{1319.41F, 7299.94F, "Qz", MoveState::CAST_SKILL, &skillbar.qz},
        Move{985.70F, 7325.54F, "Move Stairs", MoveState::WAIT},
        Move{-634.07F, 9071.42F, "Lab Stairs1", MoveState::WAIT},
        Move{-1522.58F, 10634.12F, "Lab Stairs2", MoveState::DONT_WAIT},
        Move{-2726.856F, 10239.48F, "Lab Stairs3", MoveState::WAIT},
        Move{-2828.35F, 10020.46F, "Lab Stairs4", MoveState::WAIT},
        Move{-4012.72F, 11130.53F, "Lab Stairs5", MoveState::WAIT},
        Move{-4012.72F, 11130.53F, "EoE", MoveState::CAST_SKILL, &skillbar.eoe},
        Move{-4470.48F, 11581.47F, "Lab Stairs6", MoveState::WAIT},
        Move{-5751.45F, 12746.52F, "Lab Reaper", MoveState::NONE, [&]() { TargetClosestReaper(player); }},
        Move{-5751.45F, 12746.52F, "Talk", MoveState::NONE, [&]() { TalkClosestReaper(player); }},
        Move{-5751.45F, 12746.52F, "Accept", MoveState::NONE, [&]() { AcceptChamber(); }},
        Move{-5751.45F, 12746.52F, "Restore", MoveState::NONE, [&]() { TakeRestore(); }},
        Move{-5751.45F, 12746.52F, "Escort", MoveState::DONT_WAIT, [&]() { TakeEscort(); }},
        Move{-6622.24F, 10387.12F, "Basement Stairs", MoveState::WAIT, &skillbar.eoe},
        Move{-6622.24F, 10387.12F, "EoE", MoveState::CAST_SKILL, &skillbar.eoe},
        Move{-5183.64F, 8876.31F, "Basement 1", MoveState::WAIT},
        Move{-6241.24F, 7945.73F, "Basement 2", MoveState::NONE},
        Move{-8798.22F, 5643.86F, "Basement 3", MoveState::WAIT},
        Move{-8402.00F, 4687.26F, "EoE", MoveState::CAST_SKILL, &skillbar.eoe},
        Move{-8798.22F, 5643.86F, "Basement 3", MoveState::NONE},
        Move{-7289.94F, 3283.81F, "Vale Door", MoveState::NONE},
        Move{-7846.65F, 2234.26F, "Vale Bridge", MoveState::WAIT},
        Move{-8764.08F, 2156.60F, "Vale Entry", MoveState::WAIT},
        Move{-12264.12F, 1821.18F, "Vale House", MoveState::WAIT},
        Move{-12264.12F, 1821.18F, "EoE", MoveState::CAST_SKILL, &skillbar.eoe},
        Move{-12145.44F, 1101.74F, "Vale Center", MoveState::DONT_WAIT},
        Move{-13760.19F, 358.15F, "Spirits 1", MoveState::WAIT},
        Move{-13757.90F, 2941.66F, "Spirits 2", MoveState::WAIT},
        Move{-13312.71F, 5165.071F, "Vale Reaper", MoveState::NONE, [&]() { TargetClosestReaper(player); }},
        Move{-2537.51F, 19139.91F, "To Dhuum 1", MoveState::DONT_WAIT},
        Move{-6202.59F, 18704.91F, "To Dhuum 2", MoveState::DONT_WAIT},
        Move{-9567.56F, 17288.916F, "To Dhuum 3", MoveState::DONT_WAIT},
        Move{-13127.69F, 17284.64F, "To Dhuum 4", MoveState::NONE},
    };

    GW::HookEntry MapLoaded_Entry;
    bool load_cb_triggered = false;
    GW::HookEntry GenericValue_Entry;
    bool interrupted = false;

    Damage damage;
};

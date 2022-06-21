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

class Damage : public DbActionABC
{
public:
    Damage(Player *p, DbSkillbar *s);

    RoutineState Routine() override;
    void Update() override;

private:
    bool PauseRoutine();
    bool ResumeRoutine();

    RoutineState RoutineValeSpirits() const;
    RoutineState RoutinePI(const uint32_t dhuum_id) const;
    RoutineState RoutineDhuum() const;

    GW::Agent *lt_agent = nullptr;
    GW::Agent *emo_agent = nullptr;
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
    std::array<Move, 28> moves = {
        Move{613.38F, 7097.03F, "SQ", MoveState::DONT_WAIT, [&]() { skillbar.sq.Cast(player.energy); }},
        Move{157.41F, 7781.66F, "Move EoE", MoveState::DONT_WAIT},
        Move{157.41F, 7781.66F, "EoE", MoveState::CAST_SKILL, &skillbar.eoe},
        Move{1319.41F, 7299.941F, "Move Qz", MoveState::DONT_WAIT},
        Move{1319.41F, 7299.94F, "Qz", MoveState::CAST_SKILL, &skillbar.qz},
        Move{985.70F, 7325.54F, "Move Stairs", MoveState::WAIT, 1500.0F},
        Move{-583.28F, 9275.68F, "Lab Stairs1", MoveState::WAIT, 1500.0F},
        Move{-1951.42F, 10689.08F, "Lab Stairs2", MoveState::WAIT, 1500.0F},
        Move{-2984.86F, 10626.32F, "Lab Stairs3", MoveState::WAIT, 1700.0F},
        Move{-3790.54F, 11138.82F, "Lab Stairs4", MoveState::CAST_SKILL, &skillbar.eoe},
        Move{-5751.45F, 12746.52F, "Lab Reaper", MoveState::NONE, [&]() { TargetClosestReaper(player); }},
        Move{-5751.45F, 12746.52F, "Restore", MoveState::NONE, [&]() { TakeRestore(); }},
        Move{-5751.45F, 12746.52F, "Escort", MoveState::DONT_WAIT, [&]() { TakeEscort(); }},
        Move{-6622.24F, 10387.12F, "Fuse Pull 1", MoveState::CAST_SKILL, &skillbar.eoe},
        Move{-5399.54F, 9022.45F, "Basement Stairs 1", MoveState::WAIT, 2000.0F},
        Move{-6241.24F, 7945.73F, "Basement", MoveState::WAIT, 2000.0F},
        Move{-8763.36F, 5551.18F, "Basement Stairs 2", MoveState::WAIT, 2000.0F},
        Move{-8355.68F, 4589.67F, "Fuse Pull 2", MoveState::CAST_SKILL, &skillbar.eoe},
        Move{-8764.08F, 2156.60F, "Vale Entry", MoveState::WAIT, 1800.0F},
        Move{-12264.12F, 1821.18F, "Vale House", MoveState::WAIT, 3500.0F},
        Move{-12145.44F, 1101.74F, "Vale Center", MoveState::DONT_WAIT},
        Move{-13760.19F, 358.15F, "Spirits 2", MoveState::WAIT, 1300.0F},
        Move{-13872.34F, 2332.34F, "Spirits 1", MoveState::WAIT, 1600.0F},
        Move{-13312.71F, 5165.071F, "Vale Reaper", MoveState::NONE},
        Move{-2537.51F, 19139.91F, "To Dhuum 1", MoveState::DONT_WAIT},
        Move{-6202.59F, 18704.91F, "To Dhuum 2", MoveState::DONT_WAIT},
        Move{-9567.56F, 17288.916F, "To Dhuum 3", MoveState::DONT_WAIT},
        Move{-13127.69F, 17284.64F, "To Dhuum 4", MoveState::NONE},
    };

    GW::HookEntry MapLoaded_Entry;
    bool load_cb_triggered = false;

    Damage damage;
};

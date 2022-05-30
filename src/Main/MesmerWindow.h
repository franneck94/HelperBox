#pragma once

#include "stdafx.h"

#include <cstdint>
#include <vector>

#include <GWCA/GameEntities/Agent.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Packets/Opcodes.h>

#include <Actions.h>
#include <Player.h>
#include <Timer.h>
#include <Types.h>

#include <Base/HelperBoxWindow.h>

class SpikeSet : public MesmerActionABC
{
public:
    SpikeSet(Player *p, MesmerSkillbar *s) : MesmerActionABC(p, "SpikeSet", s)
    {
    }

    RoutineState Routine() override;
    void Update() override;
};

class MesmerWindow : public HelperBoxWindow
{
public:
    MesmerWindow() : player({}), skillbar({}), spike_set(&player, &skillbar), filtered_foes({})
    {
        if (skillbar.ValidateData())
        {
            skillbar.Load();
        }
    };
    ~MesmerWindow(){};

    static MesmerWindow &Instance()
    {
        static MesmerWindow instance;
        return instance;
    }

    const char *Name() const override
    {
        return "MesmerWindow";
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
    void Spiking();

    Player player;
    MesmerSkillbar skillbar;
    std::vector<GW::AgentLiving *> filtered_foes;

    SpikeSet spike_set;
};

#pragma once

#include "stdafx.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <GWCA/GameEntities/Agent.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Packets/Opcodes.h>

#include <Actions.h>
#include <Player.h>
#include <Timer.h>
#include <Types.h>

#include <Base/HelperBoxWindow.h>

class SpikeSet : public ActionABC
{
public:
    constexpr static uint32_t demise_idx = 0;
    constexpr static uint32_t worry_idx = 1;

    SpikeSet(Player *p) : ActionABC(p, "SpikeSet")
    {
    }

    RoutineState Routine() override;
    void Update() override;
};

class SpikerWindow : public HelperBoxWindow
{
public:
    SpikerWindow() : player({}), spike_set(&player), filtered_foes({}){};
    ~SpikerWindow(){};

    static SpikerWindow &Instance()
    {
        static SpikerWindow instance;
        return instance;
    }

    const char *Name() const override
    {
        return "SpikerWindow";
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
    std::vector<GW::AgentLiving *> filtered_foes;

    SpikeSet spike_set;
};

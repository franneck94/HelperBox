#pragma once

#include "stdafx.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <GWCA/GameEntities/Agent.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Packets/Opcodes.h>

#include <Player.h>
#include <Timer.h>
#include <Types.h>

#include <Base/HelperBoxWindow.h>

class SpikerWindow : public HelperBoxWindow
{
public:
    SpikerWindow() : player(), nearby_foes({}){};
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
    std::vector<GW::AgentLiving *> nearby_foes;
};

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

class TerraWindow : public HelperBoxWindow
{
public:
    TerraWindow() : player({}), filtered_foes({}){};
    ~TerraWindow(){};

    static TerraWindow &Instance()
    {
        static TerraWindow instance;
        return instance;
    }

    const char *Name() const override
    {
        return "TerraWindow";
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
};

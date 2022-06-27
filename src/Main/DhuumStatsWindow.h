#pragma once

#include "stdafx.h"

#include <cstdint>

#include <GWCA/GameEntities/Agent.h>

#include <Actions.h>
#include <Player.h>
#include <Types.h>

#include <Base/HelperBoxWindow.h>

class DhuumStatsWindow : public HelperBoxWindow
{
public:
    DhuumStatsWindow() : player({}){};
    ~DhuumStatsWindow(){};

    static DhuumStatsWindow &Instance()
    {
        static DhuumStatsWindow instance;
        return instance;
    }

    const char *Name() const override
    {
        return "DhuumStatsWindow";
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
    Player player;
};

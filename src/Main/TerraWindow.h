#pragma once

#include "stdafx.h"

#include <cstdint>
#include <map>
#include <vector>

#include <GWCA/GameEntities/Agent.h>

#include <Player.h>
#include <Timer.h>
#include <Types.h>

#include <Base/HelperBoxWindow.h>

class AutoTargetAction : public ActionABC
{
public:
    AutoTargetAction(Player *p) : ActionABC(p, "AutoTarget")
    {
    }

    RoutineState Routine() override;
    void Update() override;
};

class TerraWindow : public HelperBoxWindow
{
public:
    TerraWindow() : player({}), filtered_foes({}), auto_target(&player), last_casted_times_ms({}){};
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
    bool TerraWindow::ActivationConditions();

    void Spiking();

    Player player;
    std::vector<GW::AgentLiving *> filtered_foes;
    std::map<uint32_t, clock_t> last_casted_times_ms;

    AutoTargetAction auto_target;
};

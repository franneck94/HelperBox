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
    AutoTargetAction(Player *p) : ActionABC(p, "Auto Target")
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
    bool ActivationConditions();
    void DrawSplittedAgents(std::vector<GW::AgentLiving *> splitted_agents, const ImVec4 color, std::string_view label);

    void Spiking();

    Player player;
    std::map<uint32_t, clock_t> last_casted_times_ms;

    std::vector<GW::AgentLiving *> filtered_foes;
    std::vector<GW::AgentLiving *> behemoth_livings;
    std::vector<GW::AgentLiving *> dryder_livings;
    std::vector<GW::AgentLiving *> skele_livings;
    std::vector<GW::AgentLiving *> horseman_livings;

    AutoTargetAction auto_target;
};

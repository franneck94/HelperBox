#pragma once

#include "stdafx.h"

#include <cstdint>
#include <vector>

#include <GWCA/GameEntities/Agent.h>

#include <Actions.h>
#include <Player.h>
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

class MainteamWindow : public HelperBoxWindow
{
public:
    MainteamWindow()
        : player({}), skillbar({}), spike_set(&player, &skillbar), filtered_foes({}), aatxe_agents({}),
          dryder_agents({}), skele_agents({})
    {
        if (skillbar.ValidateData())
        {
            skillbar.Load();
        }
    };
    ~MainteamWindow(){};

    static MainteamWindow &Instance()
    {
        static MainteamWindow instance;
        return instance;
    }

    const char *Name() const override
    {
        return "MainteamWindow";
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
    MesmerSkillbar skillbar;
    std::vector<GW::AgentLiving *> filtered_foes;
    std::vector<GW::AgentLiving *> aatxe_agents;
    std::vector<GW::AgentLiving *> dryder_agents;
    std::vector<GW::AgentLiving *> nightmare_agents;
    std::vector<GW::AgentLiving *> skele_agents;

    SpikeSet spike_set;
};

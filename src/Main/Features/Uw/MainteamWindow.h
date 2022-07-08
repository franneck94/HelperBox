#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include <GWCA/GameEntities/Agent.h>

#include <ActionsUw.h>
#include <Base/HelperBoxWindow.h>
#include <DataLivings.h>
#include <DataPlayer.h>
#include <Types.h>

#include <SimpleIni.h>
#include <imgui.h>

class MainteamWindow : public HelperBoxWindow
{
public:
    MainteamWindow()
        : player_data({}), filtered_livings({}), aatxe_livings({}), dryder_livings({}), skele_livings({}){};
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

    void Draw(IDirect3DDevice9 *pDevice) override;
    void Update(float delta, const AgentLivingData &) override;

private:
    void DrawSplittedAgents(std::vector<GW::AgentLiving *> livings, const ImVec4 color, std::string_view label);

    DataPlayer player_data;
    const AgentLivingData *livings_data = nullptr;

    std::vector<GW::AgentLiving *> filtered_livings;
    std::vector<GW::AgentLiving *> aatxe_livings;
    std::vector<GW::AgentLiving *> dryder_livings;
    std::vector<GW::AgentLiving *> nightmare_livings;
    std::vector<GW::AgentLiving *> skele_livings;
    std::vector<GW::AgentLiving *> horseman_livings;
    std::vector<GW::AgentLiving *> keeper_livings;
};

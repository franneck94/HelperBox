#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include <GWCA/GameEntities/Agent.h>

#include <PlayerData.h>
#include <Timer.h>
#include <Types.h>

#include <Base/HelperBoxWindow.h>

class AutoTargetAction : public ActionABC
{
public:
    AutoTargetAction(PlayerData *p) : ActionABC(p, "Auto Target")
    {
    }

    RoutineState Routine() override;
    void Update() override;
};

class TerraWindow : public HelperBoxWindow
{
public:
    TerraWindow() : player_data({}), filtered_livings({}), auto_target(&player_data), last_casted_times_ms({}){};
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

    void Draw(IDirect3DDevice9 *pDevice) override;
    void Update(float delta, const AgentLivingData &) override;

private:
    void DrawSplittedAgents(std::vector<GW::AgentLiving *> livings, const ImVec4 color, std::string_view label);

    PlayerData player_data;
    std::map<uint32_t, clock_t> last_casted_times_ms;

    std::vector<GW::AgentLiving *> filtered_livings;
    std::vector<GW::AgentLiving *> behemoth_livings;
    std::vector<GW::AgentLiving *> dryder_livings;
    std::vector<GW::AgentLiving *> skele_livings;
    std::vector<GW::AgentLiving *> horseman_livings;

    AutoTargetAction auto_target;
};

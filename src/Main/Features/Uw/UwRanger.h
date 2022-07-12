#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include <GWCA/GameEntities/Agent.h>

#include <ActionsBase.h>
#include <Base/HelperBoxWindow.h>
#include <DataPlayer.h>
#include <Utils.h>

#include <SimpleIni.h>
#include <imgui.h>

class AutoTargetAction : public ActionABC
{
public:
    AutoTargetAction(DataPlayer *p) : ActionABC(p, "Auto Target"){};

    RoutineState Routine() override;
    void Update() override;
};

class UwRanger : public HelperBoxWindow
{
public:
    UwRanger() : player_data({}), filtered_livings({}), auto_target(&player_data), last_casted_times_ms({}){};
    ~UwRanger(){};

    static UwRanger &Instance()
    {
        static UwRanger instance;
        return instance;
    }

    const char *Name() const override
    {
        return "UwRanger";
    }

    void Draw() override;
    void Update(float delta, const AgentLivingData &) override;

private:
    void DrawSplittedAgents(std::vector<GW::AgentLiving *> livings, const ImVec4 color, std::string_view label);

    DataPlayer player_data;
    std::map<uint32_t, clock_t> last_casted_times_ms;

    std::vector<GW::AgentLiving *> filtered_livings;
    std::vector<GW::AgentLiving *> behemoth_livings;
    std::vector<GW::AgentLiving *> dryder_livings;
    std::vector<GW::AgentLiving *> skele_livings;
    std::vector<GW::AgentLiving *> horseman_livings;

    AutoTargetAction auto_target;
};

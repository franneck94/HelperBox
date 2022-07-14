#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include <GWCA/GameEntities/Agent.h>

#include "UwHelperBase.h"
#include <ActionsBase.h>
#include <ActionsUw.h>
#include <Base/HelperBoxWindow.h>
#include <DataLivings.h>
#include <DataPlayer.h>

#include <SimpleIni.h>
#include <imgui.h>

class LtRoutine : public MesmerActionABC
{
public:
    LtRoutine(DataPlayer *p, MesmerSkillbarData *s, const AgentLivingData *a)
        : MesmerActionABC(p, "LtRoutine", s), livings_data(a){};

    RoutineState Routine() override;
    void Update() override;

private:
    static bool EnemyShouldGetEmpathy(const std::vector<GW::AgentLiving *> &enemies, const GW::AgentLiving *enemy);
    bool DoNeedVisage(const std::vector<GW::AgentLiving *> &enemies,
                      const std::vector<GW::AgentLiving *> &aatxes,
                      const std::vector<GW::AgentLiving *> &graspings) const;
    bool ReadyForSpike() const;
    bool DoNeedEnchNow(const GW::Constants::SkillID ench_id) const;
    bool RoutineSelfEnches(const std::vector<GW::AgentLiving *> &enemies) const;
    bool RoutineSpikeBall(const std::vector<GW::AgentLiving *> &enemies, const auto include_graspings);
    bool CastHexesOnEnemyType(const std::vector<GW::AgentLiving *> &enemies,
                              const std::vector<GW::AgentLiving *> &filtered_enemies,
                              uint32_t &last_skill,
                              uint32_t &last_id,
                              const bool use_empathy);

public:
    const AgentLivingData *livings_data = nullptr;
    bool load_cb_triggered = false;

private:
    uint32_t last_nightmare_id = 0U;
    uint32_t last_nightmare_skill = 0U;
    uint32_t last_aatxe_id = 0U;
    uint32_t last_aatxe_skill = 0U;
    uint32_t last_dryder_id = 0U;
    uint32_t last_dryder_skill = 0U;
    uint32_t last_graspings_id = 0U;
    uint32_t last_graspings_skill = 0U;

    bool starting_active = false;
};

class UwMesmer : public HelperBoxWindow, public UwHelperABC
{
public:
    UwMesmer()
        : player_data({}), filtered_livings({}), aatxe_livings({}), dryder_livings({}), skele_livings({}), skillbar({}),
          lt_routine(&player_data, &skillbar, livings_data)
    {
        if (skillbar.ValidateData())
            skillbar.Load();
    };
    ~UwMesmer(){};

    static UwMesmer &Instance()
    {
        static UwMesmer instance;
        return instance;
    }

    const char *Name() const override
    {
        return "UwMesmer";
    }

    void Draw() override;
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

    MesmerSkillbarData skillbar;
    LtRoutine lt_routine;
};

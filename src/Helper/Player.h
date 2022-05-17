#pragma once

#include <cstdint>

#include <GWCA/GameContainers/Array.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Skill.h>

#include <Skillbars.h>

class Player
{
public:
    Player(const uint32_t agent_id = UINT32_MAX)
        : id(agent_id), skillbar(EmoSkillbar{}), pos(GW::GamePos{0.0F, 0.0F, 0}){};

    bool ValidateData() const;
    void Update();

    bool CanCast() const;

public:
    uint32_t id = UINT32_MAX;

    EmoSkillbar skillbar;
    GW::Skillbar *internal_skillbar = nullptr;
    GW::SkillbarSkill *skillbar_skills = nullptr;

    GW::Agent *me = nullptr;
    GW::AgentLiving *living = nullptr;
    GW::Agent *target = nullptr;
    GW::GamePos pos;

    uint32_t energy = 0U;
    uint32_t max_energy = 0U;
    float energy_perc = 0.0F;

    uint32_t hp = 0U;
    uint32_t max_hp = 0U;
    float hp_perc = 0.0F;

    bool dead = false;
};
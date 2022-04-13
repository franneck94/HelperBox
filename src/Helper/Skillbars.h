#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include <GWCA/Constants/Skills.h>
#include <GWCA/GameEntities/Skill.h>
#include <GWCA/Managers/SkillbarMgr.h>

struct SkillData
{
public:
    const uint32_t id;
    const uint32_t idx;
    uint8_t energy_cost;
    uint32_t recharge;

    SkillData(const GW::Constants::SkillID id_, const uint32_t idx_)
        : id(static_cast<uint32_t>(id_)), idx(idx_), energy_cost(0U), recharge(UINT32_MAX)
    {
        energy_cost = GW::SkillbarMgr::GetSkillConstantData(id).energy_cost;
    }

    void Update(const GW::SkillbarSkill *const skillbar_skills)
    {
        recharge = skillbar_skills[idx].GetRecharge();

        if (recharge >= UINT16_MAX)
        {
            recharge = 0U;
        }
    }

    bool CanBeCasted(const uint32_t current_energy) const
    {
        return (current_energy > energy_cost && recharge == 0);
    }
};

class RangerRunnerSkillbar
{
public:
    std::string_view code = "OgcUcZsklPTHQ6M3lTQ0kNQAAAAA";

    SkillData shroud = SkillData{GW::Constants::SkillID::Shroud_of_Darkness, 0};
    SkillData sf = SkillData{GW::Constants::SkillID::Shadow_Form, 1};
    SkillData dwarfen = SkillData{GW::Constants::SkillID::Dwarven_Stability, 2};
    SkillData dash = SkillData{GW::Constants::SkillID::Dash, 3};
    SkillData iau = SkillData{GW::Constants::SkillID::I_Am_Unstoppable, 4};
    SkillData dark_escape = SkillData{GW::Constants::SkillID::Dark_Escape, 5};
    SkillData empty1 = SkillData{GW::Constants::SkillID::No_Skill, 6};
    SkillData empty2 = SkillData{GW::Constants::SkillID::No_Skill, 7};

private:
    std::array<SkillData *, 8> skills = {&shroud, &sf, &dwarfen, &dash, &iau, &dark_escape, &empty1, &empty2};

public:
    void Update(const GW::SkillbarSkill *skillbar_skills)
    {
        for (auto &skill : skills)
        {
            skill->Update(skillbar_skills);
        }
    }
};

class EmoSkillbar
{
public:
    std::string_view code = "OgNDwcPPT3MaRkE1CxDHEyD0lA";

    SkillData burning = SkillData{GW::Constants::SkillID::Burning_Speed, 0};
    SkillData sb = SkillData{GW::Constants::SkillID::Spirit_Bond, 1};
    SkillData fuse = SkillData{GW::Constants::SkillID::Infuse_Health, 2};
    SkillData ether = SkillData{GW::Constants::SkillID::Ether_Renewal, 3};
    SkillData bond = SkillData{GW::Constants::SkillID::Protective_Bond, 4};
    SkillData life = SkillData{GW::Constants::SkillID::Life_Bond, 5};
    SkillData balth = SkillData{GW::Constants::SkillID::Balthazars_Spirit, 6};
    SkillData ebon = SkillData{GW::Constants::SkillID::Ebon_Escape, 7};

private:
    std::array<SkillData *, 8> skills = {&burning, &sb, &fuse, &ether, &bond, &life, &balth, &ebon};

public:
    void Update(const GW::SkillbarSkill *skillbar_skills)
    {
        for (auto skill : skills)
        {
            skill->Update(skillbar_skills);
        }
    }
};

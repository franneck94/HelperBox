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
    uint32_t idx;
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

class SkillbarABC
{
public:
    bool ValidateData()
    {
        const auto skillbar_ = GW::SkillbarMgr::GetPlayerSkillbar();
        if (!skillbar_)
            return false;

        const auto skillbar_skills_ = skillbar_->skills;
        if (!skillbar_skills_)
            return false;

        return true;
    }

    void Load()
    {
        const auto internal_skillbar = GW::SkillbarMgr::GetPlayerSkillbar();
        const auto skillbar_skills = internal_skillbar->skills;
        LoadInternal(skillbar_skills);
    }

    virtual void LoadInternal(const GW::SkillbarSkill *skillbar_skills) = 0;

    void Update()
    {
        const auto internal_skillbar = GW::SkillbarMgr::GetPlayerSkillbar();
        const auto skillbar_skills = internal_skillbar->skills;
        UpdateInternal(skillbar_skills);
    }

    virtual void UpdateInternal(const GW::SkillbarSkill *skillbar_skills) = 0;
};

class EmoSkillbar : public SkillbarABC
{
public:
    SkillData burning = SkillData{GW::Constants::SkillID::Burning_Speed, static_cast<uint32_t>(-1)};
    SkillData sb = SkillData{GW::Constants::SkillID::Spirit_Bond, static_cast<uint32_t>(-1)};
    SkillData fuse = SkillData{GW::Constants::SkillID::Infuse_Health, static_cast<uint32_t>(-1)};
    SkillData ether = SkillData{GW::Constants::SkillID::Ether_Renewal, static_cast<uint32_t>(-1)};
    SkillData prot = SkillData{GW::Constants::SkillID::Protective_Bond, static_cast<uint32_t>(-1)};
    SkillData life = SkillData{GW::Constants::SkillID::Life_Bond, static_cast<uint32_t>(-1)};
    SkillData balth = SkillData{GW::Constants::SkillID::Balthazars_Spirit, static_cast<uint32_t>(-1)};
    SkillData gdw = SkillData{GW::Constants::SkillID::Great_Dwarf_Weapon, static_cast<uint32_t>(-1)};
    SkillData wisdom = SkillData{GW::Constants::SkillID::Ebon_Battle_Standard_of_Wisdom, static_cast<uint32_t>(-1)};

public:
    virtual void LoadInternal(const GW::SkillbarSkill *skillbar_skills) override
    {
        if (!skillbar_skills)
            return;

        for (uint32_t idx = 0; idx < 8; ++idx)
        {
            if (skillbar_skills[idx].skill_id == static_cast<uint32_t>(burning.id))
            {
                burning.idx = idx;
            }
            else if (skillbar_skills[idx].skill_id == static_cast<uint32_t>(sb.id))
            {
                sb.idx = idx;
            }
            else if (skillbar_skills[idx].skill_id == static_cast<uint32_t>(fuse.id))
            {
                fuse.idx = idx;
            }
            else if (skillbar_skills[idx].skill_id == static_cast<uint32_t>(ether.id))
            {
                ether.idx = idx;
            }
            else if (skillbar_skills[idx].skill_id == static_cast<uint32_t>(prot.id))
            {
                prot.idx = idx;
            }
            else if (skillbar_skills[idx].skill_id == static_cast<uint32_t>(life.id))
            {
                life.idx = idx;
            }
            else if (skillbar_skills[idx].skill_id == static_cast<uint32_t>(balth.id))
            {
                balth.idx = idx;
            }
            else if (skillbar_skills[idx].skill_id == static_cast<uint32_t>(gdw.id))
            {
                gdw.idx = idx;
            }
            else if (skillbar_skills[idx].skill_id == static_cast<uint32_t>(wisdom.id))
            {
                wisdom.idx = idx;
            }
        }
    }

    virtual void UpdateInternal(const GW::SkillbarSkill *skillbar_skills) override
    {
        if (!skillbar_skills)
            return;

        if (burning.idx != static_cast<uint32_t>(-1))
            burning.Update(skillbar_skills);
        if (sb.idx != static_cast<uint32_t>(-1))
            sb.Update(skillbar_skills);
        if (fuse.idx != static_cast<uint32_t>(-1))
            fuse.Update(skillbar_skills);
        if (ether.idx != static_cast<uint32_t>(-1))
            ether.Update(skillbar_skills);
        if (prot.idx != static_cast<uint32_t>(-1))
            prot.Update(skillbar_skills);
        if (life.idx != static_cast<uint32_t>(-1))
            life.Update(skillbar_skills);
        if (balth.idx != static_cast<uint32_t>(-1))
            balth.Update(skillbar_skills);
        if (gdw.idx != static_cast<uint32_t>(-1))
            gdw.Update(skillbar_skills);
        if (wisdom.idx != static_cast<uint32_t>(-1))
            wisdom.Update(skillbar_skills);
    }
};

class MesmerSkillbar : public SkillbarABC
{
public:
    SkillData demise = SkillData{GW::Constants::SkillID::Wastrels_Demise, static_cast<uint32_t>(-1)};
    SkillData worry = SkillData{GW::Constants::SkillID::Wastrels_Worry, static_cast<uint32_t>(-1)};

public:
    virtual void LoadInternal(const GW::SkillbarSkill *skillbar_skills) override
    {
        if (!skillbar_skills)
            return;

        for (uint32_t idx = 0; idx < 8; ++idx)
        {
            if (skillbar_skills[idx].skill_id == static_cast<uint32_t>(demise.id))
            {
                demise.idx = idx;
            }
            else if (skillbar_skills[idx].skill_id == static_cast<uint32_t>(worry.id))
            {
                worry.idx = idx;
            }
        }
    }

    virtual void UpdateInternal(const GW::SkillbarSkill *skillbar_skills) override
    {
        if (!skillbar_skills)
            return;

        if (demise.idx != static_cast<uint32_t>(-1))
            demise.Update(skillbar_skills);
        if (worry.idx != static_cast<uint32_t>(-1))
            worry.Update(skillbar_skills);
    }
};

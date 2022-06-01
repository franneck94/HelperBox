#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include <GWCA/Constants/Skills.h>
#include <GWCA/GameEntities/Skill.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/SkillbarMgr.h>
#include <GWCA/Managers/StoCMgr.h>
#include <GWCA/Packets/Opcodes.h>
#include <GWCA/Packets/StoC.h>
#include <GWCA/Utilities/Hook.h>

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
    }

    bool CanBeCasted(const uint32_t current_energy) const
    {
        return (current_energy > energy_cost && recharge == 0);
    }

    bool SkillFound() const
    {
        return idx != static_cast<uint32_t>(-1);
    }
};

class SkillbarABC
{
public:
    SkillbarABC()
    {
        GW::StoC::RegisterPacketCallback<GW::Packet::StoC::MapLoaded>(
            &MapLoaded_Entry,
            [this](GW::HookStatus *status, GW::Packet::StoC::MapLoaded *packet) -> void {
                UNREFERENCED_PARAMETER(status);
                UNREFERENCED_PARAMETER(packet);
                switch (GW::Map::GetInstanceType())
                {
                case GW::Constants::InstanceType::Explorable:
                    reset = true;
                    break;
                case GW::Constants::InstanceType::Outpost:
                case GW::Constants::InstanceType::Loading:
                default:
                    reset = false;
                    break;
                }
            });
    }

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

        if (reset)
        {
            Load();
            reset = false;
        }
    }

    virtual void UpdateInternal(const GW::SkillbarSkill *skillbar_skills) = 0;

    bool reset = false;
    GW::HookEntry MapLoaded_Entry;
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
    SkillData pi = SkillData{GW::Constants::SkillID::Pain_Inverter, static_cast<uint32_t>(-1)};

public:
    virtual void LoadInternal(const GW::SkillbarSkill *skillbar_skills) override
    {
        if (!skillbar_skills)
            return;

        for (uint32_t idx = 0; idx < 8; ++idx)
        {
            if (skillbar_skills[idx].skill_id == burning.id)
                burning.idx = idx;
            else if (skillbar_skills[idx].skill_id == sb.id)
                sb.idx = idx;
            else if (skillbar_skills[idx].skill_id == fuse.id)
                fuse.idx = idx;
            else if (skillbar_skills[idx].skill_id == ether.id)
                ether.idx = idx;
            else if (skillbar_skills[idx].skill_id == prot.id)
                prot.idx = idx;
            else if (skillbar_skills[idx].skill_id == life.id)
                life.idx = idx;
            else if (skillbar_skills[idx].skill_id == balth.id)
                balth.idx = idx;
            else if (skillbar_skills[idx].skill_id == gdw.id)
                gdw.idx = idx;
            else if (skillbar_skills[idx].skill_id == wisdom.id)
                wisdom.idx = idx;
            else if (skillbar_skills[idx].skill_id == pi.id)
                pi.idx = idx;
        }
    }

    virtual void UpdateInternal(const GW::SkillbarSkill *skillbar_skills) override
    {
        if (!skillbar_skills)
            return;

        if (burning.SkillFound())
            burning.Update(skillbar_skills);
        if (sb.SkillFound())
            sb.Update(skillbar_skills);
        if (fuse.SkillFound())
            fuse.Update(skillbar_skills);
        if (ether.SkillFound())
            ether.Update(skillbar_skills);
        if (prot.SkillFound())
            prot.Update(skillbar_skills);
        if (life.SkillFound())
            life.Update(skillbar_skills);
        if (balth.SkillFound())
            balth.Update(skillbar_skills);
        if (gdw.SkillFound())
            gdw.Update(skillbar_skills);
        if (wisdom.SkillFound())
            wisdom.Update(skillbar_skills);
        if (pi.SkillFound())
            pi.Update(skillbar_skills);
    }
};

class MesmerSkillbar : public SkillbarABC
{
public:
    SkillData demise = SkillData{GW::Constants::SkillID::Wastrels_Demise, static_cast<uint32_t>(-1)};
    SkillData worry = SkillData{GW::Constants::SkillID::Wastrels_Worry, static_cast<uint32_t>(-1)};
    SkillData pi = SkillData{GW::Constants::SkillID::Pain_Inverter, static_cast<uint32_t>(-1)};

public:
    virtual void LoadInternal(const GW::SkillbarSkill *skillbar_skills) override
    {
        if (!skillbar_skills)
            return;

        for (uint32_t idx = 0; idx < 8; ++idx)
        {
            if (skillbar_skills[idx].skill_id == demise.id)
                demise.idx = idx;
            else if (skillbar_skills[idx].skill_id == worry.id)
                worry.idx = idx;
            else if (skillbar_skills[idx].skill_id == pi.id)
                pi.idx = idx;
        }
    }

    virtual void UpdateInternal(const GW::SkillbarSkill *skillbar_skills) override
    {
        if (!skillbar_skills)
            return;

        if (demise.SkillFound())
            demise.Update(skillbar_skills);
        if (worry.SkillFound())
            worry.Update(skillbar_skills);
        if (pi.SkillFound())
            pi.Update(skillbar_skills);
    }
};

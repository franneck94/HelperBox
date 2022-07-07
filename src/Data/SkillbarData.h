#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include <GWCA/Constants/Skills.h>
#include <GWCA/GameEntities/Skill.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>
#include <GWCA/Managers/SkillbarMgr.h>
#include <GWCA/Managers/StoCMgr.h>
#include <GWCA/Packets/Opcodes.h>
#include <GWCA/Packets/StoC.h>
#include <GWCA/Utilities/Hook.h>

#include <HelperCallbacks.h>
#include <SkillData.h>
#include <Types.h>

template <uint32_t N>
class SkillbarDataABC
{
public:
    SkillbarDataABC()
    {
        GW::StoC::RegisterPacketCallback<GW::Packet::StoC::MapLoaded>(
            &MapLoaded_Entry,
            [this](GW::HookStatus *status, GW::Packet::StoC::MapLoaded *packet) -> void {
                reset = ExplorableLoadCallback(status, packet);
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
        if (!internal_skillbar)
            return;
        const auto skillbar_skills = internal_skillbar->skills;
        LoadInternal(skillbar_skills);
    }

    void LoadInternal(const GW::SkillbarSkill *skillbar_skills)
    {
        if (!skillbar_skills)
            return;

        for (auto skill : skills)
        {
            if (!skill)
                continue;
            skill->idx = static_cast<uint32_t>(-1);

            for (uint32_t idx = 0; idx < 8U; ++idx)
            {
                if (skillbar_skills[idx].skill_id == skill->id)
                {
                    skill->idx = idx;
                    break;
                }
            }
        }
    }

    void Update()
    {
        const auto internal_skillbar = GW::SkillbarMgr::GetPlayerSkillbar();
        if (!internal_skillbar)
            return;
        const auto skillbar_skills = internal_skillbar->skills;
        UpdateInternal(skillbar_skills);

        if (reset)
        {
            Load();
            reset = false;
        }
    }

    void UpdateInternal(const GW::SkillbarSkill *skillbar_skills)
    {
        if (!skillbar_skills)
            return;

        for (auto skill : skills)
        {
            if (!skill)
                continue;
            if (skill->SkillFound())
                skill->Update(skillbar_skills);
        }
    }

protected:
    bool reset = false;
    GW::HookEntry MapLoaded_Entry;
    std::array<SkillData *, N> skills;
};

static constexpr auto NUM_SKILLS_EMO = size_t{10U};
class EmoSkillbarData : public SkillbarDataABC<NUM_SKILLS_EMO>
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

    EmoSkillbarData() : SkillbarDataABC()
    {
        skills = {&burning, &sb, &fuse, &ether, &prot, &life, &balth, &gdw, &wisdom, &pi};
    }
};

static constexpr auto NUM_SKILLS_MESMER = size_t{10U};
class MesmerSkillbarData : public SkillbarDataABC<NUM_SKILLS_MESMER>
{
public:
    // General Mesmer Skills
    SkillData stoneflesh = SkillData{GW::Constants::SkillID::Stoneflesh_Aura, static_cast<uint32_t>(-1)};
    SkillData obsi = SkillData{GW::Constants::SkillID::Obsidian_Flesh, static_cast<uint32_t>(-1)};
    SkillData demise = SkillData{GW::Constants::SkillID::Wastrels_Demise, static_cast<uint32_t>(-1)};
    SkillData worry = SkillData{GW::Constants::SkillID::Wastrels_Worry, static_cast<uint32_t>(-1)};
    SkillData ebon = SkillData{GW::Constants::SkillID::Ebon_Escape, static_cast<uint32_t>(-1)};
    SkillData empathy = SkillData{GW::Constants::SkillID::Empathy, static_cast<uint32_t>(-1)};

    // Solo LT Skills
    SkillData mantra_of_concentration =
        SkillData{GW::Constants::SkillID::Mantra_of_Concentration, static_cast<uint32_t>(-1)};
    SkillData visage = SkillData{GW::Constants::SkillID::Sympathetic_Visage, static_cast<uint32_t>(-1)};

    // T4 Skills
    SkillData mantra_of_earth = SkillData{GW::Constants::SkillID::Mantra_of_Earth, static_cast<uint32_t>(-1)};
    SkillData stonestriker = SkillData{GW::Constants::SkillID::Stone_Striker, static_cast<uint32_t>(-1)};

    MesmerSkillbarData() : SkillbarDataABC()
    {
        skills = {&stoneflesh,
                  &obsi,
                  &demise,
                  &worry,
                  &ebon,
                  &empathy,
                  &mantra_of_concentration,
                  &visage,
                  &mantra_of_earth,
                  &stonestriker};
    }

public:
};

static constexpr auto NUM_SKILLS_DB = size_t{8U};
class DbSkillbarData : public SkillbarDataABC<NUM_SKILLS_DB>
{
public:
    // General DB Skills
    SkillData honor = SkillData{GW::Constants::SkillID::Ebon_Battle_Standard_of_Honor, static_cast<uint32_t>(-1)};
    SkillData eoe = SkillData{GW::Constants::SkillID::Edge_of_Extinction, static_cast<uint32_t>(-1)};
    SkillData qz = SkillData{GW::Constants::SkillID::Quickening_Zephyr, static_cast<uint32_t>(-1)};
    SkillData winnow = SkillData{GW::Constants::SkillID::Winnowing, static_cast<uint32_t>(-1)};
    SkillData pi = SkillData{GW::Constants::SkillID::Pain_Inverter, static_cast<uint32_t>(-1)};

    // Only Rit Skills
    SkillData sos = SkillData{GW::Constants::SkillID::Signet_of_Spirits, static_cast<uint32_t>(-1)};
    SkillData sq = SkillData{GW::Constants::SkillID::Serpents_Quickness, static_cast<uint32_t>(-1)};
    SkillData vamp = SkillData{GW::Constants::SkillID::Vampirism, static_cast<uint32_t>(-1)};

    DbSkillbarData() : SkillbarDataABC()
    {
        skills = {&sos, &honor, &eoe, &qz, &winnow, &pi, &sq, &vamp};
    }
};

#include <cstdint>

#include <GWCA/Constants/Skills.h>
#include <GWCA/GameEntities/Skill.h>
#include <GWCA/Managers/GameThreadMgr.h>
#include <GWCA/Managers/SkillbarMgr.h>

#include "SkillData.h"

SkillData::SkillData(const GW::Constants::SkillID id_, const uint32_t idx_)
    : id(static_cast<uint32_t>(id_)), idx(idx_), energy_cost(0U), recharge(UINT32_MAX)
{
    const auto skill_data = GW::SkillbarMgr::GetSkillConstantData(id);
    if (!skill_data)
        energy_cost = 0U;
    else
        energy_cost = skill_data->energy_cost;
}

void SkillData::Update(const GW::SkillbarSkill *const skillbar_skills)
{
    if (!skillbar_skills)
        return;
    recharge = skillbar_skills[idx].GetRecharge();
}

bool SkillData::CanBeCasted(const uint32_t current_energy) const
{
    return SkillFound() && (current_energy > energy_cost && recharge == 0);
}

RoutineState SkillData::Cast(const uint32_t current_energy, const uint32_t target_id) const
{
    if (!CanBeCasted(current_energy))
        return RoutineState::ACTIVE;

    if (target_id != 0)
        GW::GameThread::Enqueue([&]() { GW::SkillbarMgr::UseSkill(idx, target_id); });
    else
        GW::GameThread::Enqueue([&]() { GW::SkillbarMgr::UseSkill(idx); });

    return RoutineState::FINISHED;
}

bool SkillData::SkillFound() const
{
    return idx != static_cast<uint32_t>(-1);
}

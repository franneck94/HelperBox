#include <utility>

#include <GWCA/Constants/Constants.h>
#include <GWCA/GameContainers/Array.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Party.h>
#include <GWCA/GameEntities/Skill.h>
#include <GWCA/Managers/EffectMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>
#include <GWCA/Managers/PartyMgr.h>

#include <Actions.h>
#include <Helper.h>
#include <HelperAgents.h>
#include <SkillbarData.h>

#include "PlayerData.h"

bool PlayerData::ValidateData(std::function<bool()> cb_fn) const
{
    if (!cb_fn())
        return false;

    const auto me_agent = GW::Agents::GetPlayer();
    const auto me_living = GW::Agents::GetPlayerAsAgentLiving();

    if (me_agent == nullptr || me_living == nullptr)
        return false;

    return true;
}

void PlayerData::Update()
{
    const auto me_agent = GW::Agents::GetPlayer();
    const auto me_living = GW::Agents::GetPlayerAsAgentLiving();

    id = me_agent->agent_id;
    pos = me_living->pos;

    me = me_agent;
    living = me_living;

    dead = living->GetIsDead();

    target = GW::Agents::GetTarget();

    const auto energy_tpl = GetEnergy();
    energy = std::get<0>(energy_tpl);
    max_energy = std::get<1>(energy_tpl);
    energy_perc = std::get<2>(energy_tpl);

    const auto hp_tpl = GetHp();
    hp = std::get<0>(hp_tpl);
    max_hp = std::get<1>(hp_tpl);
    hp_perc = std::get<2>(hp_tpl);

    primary = static_cast<GW::Constants::Profession>(living->primary);
    secondary = static_cast<GW::Constants::Profession>(living->secondary);
}

bool PlayerData::CanCast() const
{
    if (living->GetIsDead() || living->GetIsKnockedDown() || living->GetIsCasting() || living->GetIsMoving())
        return false;

    return true;
}

bool PlayerData::CanAttack() const
{
    if (living->GetIsDead() || living->GetIsKnockedDown() || living->GetIsCasting() || living->GetIsMoving())
        return false;

    return true;
}

bool PlayerData::HasBuff(const GW::Constants::SkillID buff_skill_id) const
{
    const auto me_buffs = GW::Effects::GetPlayerBuffs();
    if (!me_buffs || !me_buffs->valid())
        return false;

    for (const auto buff : *me_buffs)
    {
        const auto agent_id = buff.target_agent_id;
        const auto skill_id = buff.skill_id;

        if (agent_id == id)
        {
            if (skill_id == static_cast<uint32_t>(buff_skill_id))
                return true;
        }
    }

    return false;
}

bool PlayerData::HasEffect(const GW::Constants::SkillID effect_skill_id) const
{
    const auto me_effects = GW::Effects::GetPlayerEffectsArray();
    if (!me_effects)
        return false;

    for (const auto effect : me_effects->effects)
    {
        const auto agent_id = effect.agent_id;
        const auto skill_id = effect.skill_id;

        if (agent_id == id || agent_id == 0)
        {
            if (skill_id == static_cast<uint32_t>(effect_skill_id))
                return true;
        }
    }

    return false;
}

bool PlayerData::CastEffectIfNotAvailable(const SkillData &skill_data)
{
    const auto has_bond = HasEffect(static_cast<GW::Constants::SkillID>(skill_data.id));
    const auto bond_avail = skill_data.CanBeCasted(energy);

    if (!has_bond && bond_avail)
    {
        GW::GameThread::Enqueue([&]() { GW::SkillbarMgr::UseSkill(skill_data.idx, id); });
        return true;
    }

    return false;
}

bool PlayerData::SpamEffect(const SkillData &skill_data)
{
    if (skill_data.CanBeCasted(energy))
    {
        GW::GameThread::Enqueue([&]() { GW::SkillbarMgr::UseSkill(skill_data.idx, id); });
        return true;
    }

    return false;
}

void PlayerData::ChangeTarget(const uint32_t target_id)
{
    if (!GW::Agents::GetAgentByID(target_id))
        return;

    GW::GameThread::Enqueue([this, target_id]() {
        GW::Agents::ChangeTarget(target_id);
        target = GW::Agents::GetTarget();
    });
}


bool PlayerData::SkillStoppedCallback(GW::Packet::StoC::GenericValue *packet)
{
    const auto value_id = packet->Value_id;
    const auto caster_id = packet->agent_id;

    if (caster_id != id)
        return false;

    if (value_id == GW::Packet::StoC::GenericValueID::skill_stopped)
        return true;

    return false;
}

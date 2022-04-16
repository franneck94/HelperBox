#include <utility>

#include <GWCA/Constants/Constants.h>
#include <GWCA/GameContainers/Array.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Party.h>
#include <GWCA/GameEntities/Skill.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Managers/SkillbarMgr.h>

#include <Helper.h>

#include "Player.h"

bool Player::ValidateData() const
{
    if (IsLoading())
        return false;

    const auto me_agent = GW::Agents::GetPlayer();
    const auto me_living = GW::Agents::GetPlayerAsAgentLiving();

    if (me_agent == nullptr || me_living == nullptr)
        return false;

    if (me_agent->agent_id == UINT32_MAX)
        return false;

    if (IsOutpost())
        return true;

    const auto skillbar_ = GW::SkillbarMgr::GetPlayerSkillbar();
    if (!skillbar_)
        return false;

    const auto skillbar_skills_ = skillbar_->skills;
    if (!skillbar_skills_)
        return false;

    return true;
}

void Player::Update()
{
    const auto me_agent = GW::Agents::GetPlayer();
    const auto me_living = GW::Agents::GetPlayerAsAgentLiving();
    id = me_agent->agent_id;
    pos = me_living->pos;

    me = me_agent;
    living = me_living;

    dead = me_living->GetIsDead();

    const auto target_agent = GW::Agents::GetTarget();
    target = target_agent;

    internal_skillbar = GW::SkillbarMgr::GetPlayerSkillbar();
    skillbar_skills = internal_skillbar->skills;
    skillbar.Update(skillbar_skills);

    const auto energy_tpl = GetEnergy();
    energy = std::get<0>(energy_tpl);
    max_energy = std::get<1>(energy_tpl);
    energy_perc = std::get<2>(energy_tpl);

    const auto hp_tpl = GetHp();
    hp = std::get<0>(hp_tpl);
    max_hp = std::get<1>(hp_tpl);
    hp_perc = std::get<2>(hp_tpl);
}

bool Player::CanCast() const
{
    if (living->GetIsKnockedDown() || (internal_skillbar && internal_skillbar->casting))
    {
        return false;
    }

    return true;
}

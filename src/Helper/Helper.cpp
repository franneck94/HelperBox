#include <cmath>
#include <random>
#include <type_traits>
#include <utility>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/GameContainers/Array.h>
#include <GWCA/GameEntities/Item.h>
#include <GWCA/GameEntities/Skill.h>
#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/EffectMgr.h>
#include <GWCA/Managers/ItemMgr.h>
#include <GWCA/Managers/MemoryMgr.h>
#include <GWCA/Managers/PartyMgr.h>

#include <fmt/format.h>

#include <Logger.h>
#include <Timer.h>

#include <Actions.h>

#include "Helper.h"

bool IsLoading()
{
    return GW::Map::GetInstanceType() == GW::Constants::InstanceType::Loading;
}

bool IsExplorable()
{
    return GW::Map::GetInstanceType() == GW::Constants::InstanceType::Explorable;
}

bool IsOutpost()
{
    return GW::Map::GetInstanceType() == GW::Constants::InstanceType::Outpost;
}

bool IsMapReady()
{
    return (!IsLoading() && !GW::Map::GetIsObserving());
}

GW::EffectArray *GetEffects(const uint32_t agent_id)
{
    GW::AgentEffectsArray agent_effects = GW::Effects::GetPartyEffectArray();

    if (!agent_effects.valid())
    {
        return nullptr;
    }

    for (size_t i = 0; i < agent_effects.size(); i++)
    {
        if (agent_effects[i].agent_id == agent_id && agent_effects[i].effects.valid())
        {
            return &agent_effects[i].effects;
        }
    }

    return nullptr;
}

bool TargetNearest(const TargetType type, const float max_distance)
{
    const GW::AgentArray agents = GW::Agents::GetAgentArray();
    if (!agents.valid())
    {
        return false;
    }

    const GW::AgentLiving *const me = GW::Agents::GetPlayerAsAgentLiving();
    if (me == nullptr)
    {
        return false;
    }

    float distance = max_distance;
    size_t closest = 0;

    for (const GW::Agent *agent : agents)
    {
        if (!agent || agent == me)
        {
            continue;
        }

        switch (type)
        {
        case TargetType::Gadget:
        {
            const GW::AgentGadget *const gadget = agent->GetAsAgentGadget();
            if (!gadget)
            {
                continue;
            }
            break;
        }
        case TargetType::Item:
        {
            const GW::AgentItem *const item_agent = agent->GetAsAgentItem();
            if (!item_agent)
            {
                continue;
            }
            const GW::Item *const item = GW::Items::GetItemById(item_agent->item_id);
            if (!item)
            {
                continue;
            }
            break;
        }
        case TargetType::Npc:
        {
            const GW::AgentLiving *const living_agent = agent->GetAsAgentLiving();
            if (!living_agent || !living_agent->IsNPC() || !living_agent->GetIsAlive())
            {
                continue;
            }
            break;
        }
        case TargetType::Player:
        {
            const GW::AgentLiving *const living_agent = agent->GetAsAgentLiving();
            if (!living_agent || !living_agent->IsPlayer())
            {
                continue;
            }
            break;
        }
        case TargetType::Living:
        {
            const GW::AgentLiving *const living_agent = agent->GetAsAgentLiving();
            if (!living_agent || !living_agent->GetIsAlive())
            {
                continue;
            }
            break;
        }
        default:
        {
            continue;
        }
        }

        const float newDistance = GW::GetSquareDistance(me->pos, agent->pos);
        if (newDistance < distance)
        {
            closest = agent->agent_id;
            distance = newDistance;
        }
    }

    if (closest)
    {
        GW::Agents::ChangeTarget(closest);

        return true;
    }

    return false;
}

bool DetectPlayerIsDead()
{
    const auto me = GW::Agents::GetPlayer();

    if (nullptr == me)
    {
        return false;
    }

    const auto living_me = me->GetAsAgentLiving();

    if (nullptr == living_me)
    {
        return false;
    }

    return living_me->GetIsDead();
}

bool DetectNotMoving(const uint32_t threshold)
{
    static auto not_moving_counter = uint32_t{0};

    if (!IsMapReady())
    {
        return false;
    }

    const auto me = GW::Agents::GetPlayer();

    if (nullptr == me)
    {
        return false;
    }

    const auto living_me = me->GetAsAgentLiving();

    if (nullptr == living_me)
    {
        return false;
    }

    if (!living_me->GetIsMoving())
    {
        ++not_moving_counter;
    }

    if (not_moving_counter == threshold)
    {
        not_moving_counter = 0;
        return true;
    }
    else
    {
        return false;
    }
}

std::tuple<uint32_t, uint32_t, float> GetEnergy()
{
    const auto me = GW::Agents::GetPlayer();

    if (nullptr == me)
    {
        return std::make_tuple(0, 0, 0.0F);
    }

    const auto living_me = me->GetAsAgentLiving();

    if (nullptr == living_me)
    {
        return std::make_tuple(0, 0, 0.0F);
    }

    const auto max_energy = living_me->max_energy;
    const auto energy_perc = living_me->energy;
    const auto energy = static_cast<float>(max_energy) * energy_perc;

    return std::make_tuple(static_cast<uint32_t>(energy), max_energy, energy_perc);
}

std::tuple<uint32_t, uint32_t, float> GetHp()
{
    const auto me = GW::Agents::GetPlayer();

    if (nullptr == me)
    {
        return std::make_tuple(0, 0, 0.0F);
    }

    const auto living_me = me->GetAsAgentLiving();

    if (nullptr == living_me)
    {
        return std::make_tuple(0, 0, 0.0F);
    }

    const auto max_hp = living_me->max_hp;
    const auto hp_perc = living_me->hp;
    const auto hp = static_cast<float>(max_hp) * hp_perc;

    return std::make_tuple(static_cast<uint32_t>(hp), max_hp, hp_perc);
}

bool AgentHasBuff(const GW::Constants::SkillID buff_skill_id, const uint32_t target_agent_id)
{
    const GW::AgentEffectsArray &effects = GW::Effects::GetPartyEffectArray();

    if (!effects.valid())
    {
        return false;
    }

    const auto &buffs = effects[0].buffs;

    if (!buffs.valid())
    {
        return false;
    }

    for (size_t i = 0; i < buffs.size(); ++i)
    {
        const auto agent_id = buffs[i].target_agent_id;
        const auto skill_id = buffs[i].skill_id;

        if (agent_id == target_agent_id)
        {
            if (skill_id == static_cast<uint32_t>(buff_skill_id))
            {
                return true;
            }
        }
    }

    return false;
}

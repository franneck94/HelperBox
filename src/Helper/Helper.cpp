#include <cmath>
#include <random>
#include <type_traits>
#include <utility>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/GameContainers/Array.h>
#include <GWCA/GameEntities/Item.h>
#include <GWCA/GameEntities/Party.h>
#include <GWCA/GameEntities/Player.h>
#include <GWCA/GameEntities/Skill.h>
#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/EffectMgr.h>
#include <GWCA/Managers/ItemMgr.h>
#include <GWCA/Managers/MemoryMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Packets/Opcodes.h>

#include <fmt/format.h>

#include <Logger.h>
#include <Timer.h>

#include <Actions.h>
#include <Player.h>

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

bool GetPartyMembers(std::vector<PlayerMapping> &party_members)
{
    if (!GW::PartyMgr::GetIsPartyLoaded())
        return false;
    if (!GW::Map::GetIsMapLoaded())
        return false;

    GW::PartyInfo *info = GW::PartyMgr::GetPartyInfo();
    if (info == nullptr)
        return false;

    GW::PlayerArray players = GW::Agents::GetPlayerArray();
    if (!players.valid())
        return false;

    party_members.clear();

    uint32_t idx = 0;
    for (GW::PlayerPartyMember &player : info->players)
    {
        uint32_t id = players[player.login_number].agent_id;
        party_members.push_back({id, idx});
        ++idx;

        for (GW::HeroPartyMember &hero : info->heroes)
        {
            if (hero.owner_player_id == player.login_number)
            {
                party_members.push_back({hero.agent_id, idx});
                ++idx;
            }
        }
    }
    for (GW::HenchmanPartyMember &hench : info->henchmen)
    {
        party_members.push_back({hench.agent_id, idx});
        ++idx;
    }

    return true;
}

bool IsEquippable(const GW::Item *_item)
{
    if (!_item)
        return false;
    switch (static_cast<GW::Constants::ItemType>(_item->type))
    {
    case GW::Constants::ItemType::Axe:
    case GW::Constants::ItemType::Boots:
    case GW::Constants::ItemType::Bow:
    case GW::Constants::ItemType::Chestpiece:
    case GW::Constants::ItemType::Offhand:
    case GW::Constants::ItemType::Gloves:
    case GW::Constants::ItemType::Hammer:
    case GW::Constants::ItemType::Headpiece:
    case GW::Constants::ItemType::Leggings:
    case GW::Constants::ItemType::Wand:
    case GW::Constants::ItemType::Shield:
    case GW::Constants::ItemType::Staff:
    case GW::Constants::ItemType::Sword:
    case GW::Constants::ItemType::Daggers:
    case GW::Constants::ItemType::Scythe:
    case GW::Constants::ItemType::Spear:
    case GW::Constants::ItemType::Costume_Headpiece:
    case GW::Constants::ItemType::Costume:
        break;
    default:
        return false;
        break;
    }
    return true;
}

bool EquipItemExecute(const uint32_t bag_idx, const uint32_t slot_idx)
{
    GW::Item *item = nullptr;

    if (bag_idx < 1 || bag_idx > 5 || slot_idx < 1 || slot_idx > 25)
    {
        return false;
    }
    GW::Bag *b = GW::Items::GetBag(bag_idx);
    if (!b)
    {
        return false;
    }
    GW::ItemArray items = b->items;
    if (!items.valid() || slot_idx > items.size())
    {
        return false;
    }
    item = items.at(slot_idx - 1);

    if (!IsEquippable(item))
    {
        return false;
    }

    if (!item || !item->item_id)
    {
        return false;
    }
    if (item->bag && item->bag->bag_type == 2)
    {
        return false;
    }
    GW::AgentLiving *p = GW::Agents::GetCharacter();
    if (!p || p->GetIsDead())
    {
        return false;
    }
    const GW::Skillbar *s = GW::SkillbarMgr::GetPlayerSkillbar();
    if (p->GetIsKnockedDown() || (s && s->casting))
    {
        return false;
    }
    if (p->skill)
    {
        GW::CtoS::SendPacket(0x4, GAME_CMSG_CANCEL_MOVEMENT);
        return false;
    }
    if (p->GetIsIdle() || p->GetIsMoving())
    {
        GW::Items::EquipItem(item);
        return true;
    }
    else
    {
        GW::Agents::Move(p->pos);
        return false;
    }
}

void ChangeFullArmor(const uint32_t bag_idx, const uint32_t start_slot_idx)
{
    bool all_done = true;

    for (uint32_t offset = 0; offset < 5; offset++)
    {
        all_done = EquipItemExecute(bag_idx, start_slot_idx + offset);
    }

    if (!all_done)
    {
        Log::Log("ERROR!");
    }
}

void FilterAgents(const Player &player,
                  const GW::AgentArray &agents,
                  std::vector<GW::AgentLiving *> &filtered_agents,
                  const int id,
                  const float max_distance)
{
    for (const auto &agent : agents)
    {
        if (!agent)
            continue;

        const auto living = agent->GetAsAgentLiving();

        if (!living)
            continue;


        if (living->allegiance == static_cast<uint8_t>(GW::Constants::Allegiance::Enemy) && living->player_number == id)
        {
            if (max_distance == 0.0F)
            {
                filtered_agents.push_back(living);
            }
            else
            {
                const auto dist = GW::GetDistance(player.pos, agent->pos);

                if (dist < max_distance)
                {
                    filtered_agents.push_back(living);
                }
            }
        }
    }
}

void SortByDistance(const Player &player, std::vector<GW::AgentLiving *> &filtered_agents)
{
    const auto player_pos = player.pos;

    std::sort(filtered_agents.begin(), filtered_agents.end(), [&player_pos](auto &v1, auto &v2) {
        const auto sqrd1 = GW::GetSquareDistance(player_pos, v1->pos);
        const auto sqrd2 = GW::GetSquareDistance(player_pos, v2->pos);

        return sqrd1 < sqrd2;
    });
}

bool IsAtDhuumFight(const Player *player)
{
    if (GW::Map::GetMapID() != GW::Constants::MapID::The_Underworld)
        return false;

    const auto dhuum_center_pos = GW::GamePos{-16105.50F, 17284.84F, player->pos.zplane};
    const auto dhuum_center_dist = GW::GetDistance(player->pos, dhuum_center_pos);

    if (dhuum_center_dist > GW::Constants::Range::Spellcast)
        return false;

    auto agents_array = GW::Agents::GetAgentArray();
    if (agents_array.size() < 2)
        return false;

    const auto it = std::find_if(agents_array.begin(), agents_array.end(), [](const auto agent) {
        return agent->agent_id == GW::Constants::ModelID::UW::Dhuum;
    });

    if (it != agents_array.end())
    {
        const auto dhuum_living = (*it)->GetAsAgentLiving();
        if (!dhuum_living)
            return false;

        if (dhuum_living->allegiance != static_cast<uint8_t>(GW::Constants::Allegiance::Ally_NonAttackable))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
}

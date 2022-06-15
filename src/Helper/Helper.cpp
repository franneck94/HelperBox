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

static bool _IsEndGameEntryOutpost()
{
    return (
#ifdef _DEBUG
        GW::Map::GetMapID() == GW::Constants::MapID::Great_Temple_of_Balthazar_outpost ||
        GW::Map::GetMapID() == GW::Constants::MapID::Isle_of_the_Nameless ||
#endif
        GW::Map::GetMapID() == GW::Constants::MapID::Embark_Beach ||
        GW::Map::GetMapID() == GW::Constants::MapID::Temple_of_the_Ages ||
        GW::Map::GetMapID() == GW::Constants::MapID::Chantry_of_Secrets_outpost ||
        GW::Map::GetMapID() == GW::Constants::MapID::Zin_Ku_Corridor_outpost);
}

bool IsUwEntryOutpost()
{
    return _IsEndGameEntryOutpost();
}

bool IsFowEntryOutpost()
{
    return _IsEndGameEntryOutpost();
}

bool IsFow()
{
    return (GW::Map::GetMapID() == GW::Constants::MapID::The_Fissure_of_Woe);
}

bool IsDoa()
{
    return (GW::Map::GetMapID() == GW::Constants::MapID::Domain_of_Anguish);
}

bool IsDoaEntryOutpost()
{
    return (GW::Map::GetMapID() == GW::Constants::MapID::Gate_of_Torment_outpost);
}

const GW::EffectArray *GetEffects(const uint32_t agent_id)
{
    const auto agent_effects = GW::Effects::GetPartyEffectArray();

    if (!agent_effects.valid())
        return nullptr;

    for (size_t i = 0; i < agent_effects.size(); i++)
    {
        if (agent_effects[i].agent_id == agent_id && agent_effects[i].effects.valid())
            return &agent_effects[i].effects;
    }

    return nullptr;
}

bool TargetNearest(const TargetType type, const float max_distance)
{
    const auto agents = GW::Agents::GetAgentArray();
    if (!agents.valid())
        return false;

    const auto me = GW::Agents::GetPlayerAsAgentLiving();
    if (!me)
        return false;

    auto distance = max_distance;
    auto closest = size_t{0};

    for (const auto agent : agents)
    {
        if (!agent || agent == me)
            continue;

        switch (type)
        {
        case TargetType::Gadget:
        {
            const auto gadget = agent->GetAsAgentGadget();
            if (!gadget)
                continue;
            break;
        }
        case TargetType::Item:
        {
            const auto item_agent = agent->GetAsAgentItem();
            if (!item_agent)
                continue;

            const auto item = GW::Items::GetItemById(item_agent->item_id);
            if (!item)
                continue;
            break;
        }
        case TargetType::Npc:
        {
            const auto living_agent = agent->GetAsAgentLiving();
            if (!living_agent || !living_agent->IsNPC() || !living_agent->GetIsAlive())
                continue;
            break;
        }
        case TargetType::Player:
        {
            const auto living_agent = agent->GetAsAgentLiving();
            if (!living_agent || !living_agent->IsPlayer())
                continue;
            break;
        }
        case TargetType::Living:
        {
            const auto living_agent = agent->GetAsAgentLiving();
            if (!living_agent || !living_agent->GetIsAlive())
                continue;
            break;
        }
        default:
        {
            continue;
        }
        }

        const auto newDistance = GW::GetSquareDistance(me->pos, agent->pos);
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
    if (!me)
        return false;

    const auto living_me = me->GetAsAgentLiving();
    if (!living_me)
        return false;

    return living_me->GetIsDead();
}

std::tuple<uint32_t, uint32_t, float> GetEnergy()
{
    const auto me = GW::Agents::GetPlayer();
    if (!me)
        return std::make_tuple(0, 0, 0.0F);

    const auto living_me = me->GetAsAgentLiving();
    if (!living_me)
        return std::make_tuple(0, 0, 0.0F);

    const auto max_energy = living_me->max_energy;
    const auto energy_perc = living_me->energy;
    const auto energy = static_cast<float>(max_energy) * energy_perc;

    return std::make_tuple(static_cast<uint32_t>(energy), max_energy, energy_perc);
}

std::tuple<uint32_t, uint32_t, float> GetHp()
{
    const auto me = GW::Agents::GetPlayer();
    if (!me)
        return std::make_tuple(0, 0, 0.0F);

    const auto living_me = me->GetAsAgentLiving();
    if (!living_me)
        return std::make_tuple(0, 0, 0.0F);

    const auto max_hp = living_me->max_hp;
    const auto hp_perc = living_me->hp;
    const auto hp = static_cast<float>(max_hp) * hp_perc;

    return std::make_tuple(static_cast<uint32_t>(hp), max_hp, hp_perc);
}

bool AgentHasBuff(const GW::Constants::SkillID buff_skill_id, const uint32_t target_agent_id)
{
    const auto &effects = GW::Effects::GetPartyEffectArray();
    if (!effects.valid())
        return false;

    const auto &buffs = effects[0].buffs;
    if (!buffs.valid())
        return false;

    for (size_t i = 0; i < buffs.size(); ++i)
    {
        const auto agent_id = buffs[i].target_agent_id;
        const auto skill_id = buffs[i].skill_id;

        if (agent_id == target_agent_id)
        {
            if (skill_id == static_cast<uint32_t>(buff_skill_id))
                return true;
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

    const auto info = GW::PartyMgr::GetPartyInfo();
    if (!info)
        return false;

    const auto players = GW::Agents::GetPlayerArray();
    if (!players.valid())
        return false;

    party_members.clear();

    auto idx = uint32_t{0};
    for (const auto &player : info->players)
    {
        const auto id = players[player.login_number].agent_id;
        party_members.push_back({id, idx});
        ++idx;

        for (const auto &hero : info->heroes)
        {
            if (hero.owner_player_id == player.login_number)
            {
                party_members.push_back({hero.agent_id, idx});
                ++idx;
            }
        }
    }
    for (const auto &hench : info->henchmen)
    {
        party_members.push_back({hench.agent_id, idx});
        ++idx;
    }

    return true;
}

bool IsEquippable(const GW::Item *item)
{
    if (!item)
        return false;

    switch (static_cast<GW::Constants::ItemType>(item->type))
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
        return false;

    const auto b = GW::Items::GetBag(bag_idx);
    if (!b)
        return false;

    auto items = b->items;
    if (!items.valid() || slot_idx > items.size())
        return false;
    item = items.at(slot_idx - 1);

    if (!IsEquippable(item))
        return false;

    if (!item || !item->item_id)
        return false;

    if (item->bag && item->bag->bag_type == 2)
        return false;

    const auto p = GW::Agents::GetCharacter();
    if (!p || p->GetIsDead())
        return false;

    const auto s = GW::SkillbarMgr::GetPlayerSkillbar();
    if (p->GetIsKnockedDown() || (s && s->casting))
        return false;

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
    if (static_cast<uint32_t>(-1) == bag_idx || static_cast<uint32_t>(-1) == start_slot_idx)
        return;

    for (uint32_t offset = 0; offset < 5; offset++)
        EquipItemExecute(bag_idx, start_slot_idx + offset);
}

void SortByDistance(const Player &player, std::vector<GW::AgentLiving *> &filtered_agents)
{
    const auto player_pos = player.pos;

    std::sort(filtered_agents.begin(), filtered_agents.end(), [&player_pos](const auto a1, const auto a2) {
        const auto sqrd1 = GW::GetSquareDistance(player_pos, a1->pos);
        const auto sqrd2 = GW::GetSquareDistance(player_pos, a2->pos);
        return sqrd1 < sqrd2;
    });
}

bool CanMove()
{
    return !IsLoading() && !GW::Map::GetIsObserving() && GW::MemoryMgr::GetGWWindowHandle() == GetActiveWindow();
}

uint32_t GetTankId()
{
    std::vector<PlayerMapping> party_members;
    const auto success = GetPartyMembers(party_members);

    if (!success || party_members.size() < 2)
        return 0;

    auto tank_idx = uint32_t{0};

    switch (GW::Map::GetMapID())
    {
    case GW::Constants::MapID::The_Underworld:
#ifdef _DEBUG
    case GW::Constants::MapID::Isle_of_the_Nameless:
#endif
    {
        tank_idx = party_members.size() - 2;
        break;
    }
    default:
    {
        tank_idx = 0;
        break;
    }
    }

    const auto tank = party_members[tank_idx];
    return tank.id;
}

bool IsAliveAlly(const GW::Agent *target)
{
    if (!target)
        return false;

    if (!target->GetIsLivingType())
        return false;

    const auto target_living = target->GetAsAgentLiving();
    if (!target_living)
        return false;

    if (target_living->allegiance != static_cast<uint8_t>(GW::Constants::Allegiance::Ally_NonAttackable) ||
        target_living->GetIsDead())
        return false;

    return true;
}

uint32_t GetPartyIdxByID(const uint32_t id)
{
    std::vector<PlayerMapping> party_members;
    const auto success = GetPartyMembers(party_members);

    if (!success)
        return 0U;

    const auto it = std::find_if(party_members.begin(), party_members.end(), [&id](const PlayerMapping &member) {
        return member.id == static_cast<uint32_t>(id);
    });
    if (it == party_members.end())
        return 0U;

    const auto idx = static_cast<uint32_t>(std::distance(party_members.begin(), it));
    if (idx >= GW::PartyMgr::GetPartySize())
        return 0U;

    return idx;
}

void SplitFilteredAgents(const std::vector<GW::AgentLiving *> &filtered_agents,
                         std::vector<GW::AgentLiving *> &splitted_agents,
                         const uint32_t id)
{
    for (auto agent : filtered_agents)
    {
        if (agent->player_number == id)
            splitted_agents.push_back(agent);
    }
}

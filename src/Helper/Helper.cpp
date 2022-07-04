#include <cmath>
#include <iterator>
#include <random>
#include <set>
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
#include <HelperPlayer.h>
#include <PlayerData.h>

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

bool IsEndGameEntryOutpost()
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

bool IsFowEntryOutpost()
{
    return IsEndGameEntryOutpost();
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

bool HelperActivationConditions()
{
    if (!GW::Map::GetIsMapLoaded())
        return false;

    if (!GW::PartyMgr::GetIsPartyLoaded())
        return false;

    if (!IsMapReady())
        return false;

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

GW::Item *GetBagItem(const uint32_t bag_idx, const uint32_t slot_idx)
{
    GW::Item *item = nullptr;

    if (bag_idx < 1 || bag_idx > 5 || slot_idx < 1 || slot_idx > 25)
        return nullptr;

    const auto bags = GW::Items::GetBagArray();
    if (!bags)
        return nullptr;
    const auto bag = bags[bag_idx];
    if (!bag)
        return nullptr;

    auto &items = bag->items;
    if (!items.valid() || slot_idx > items.size())
        return nullptr;
    item = items.at(slot_idx - 1);

    return item;
}

bool EquipItemExecute(const uint32_t bag_idx, const uint32_t slot_idx)
{
    const auto item = GetBagItem(bag_idx, slot_idx);
    if (!item)
        return false;

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

bool ArmorSwap(const uint32_t bag_idx, const uint32_t start_slot_idx, const bool low_armor)
{
    if (static_cast<uint32_t>(-1) == bag_idx || static_cast<uint32_t>(-1) == start_slot_idx)
        return true;

    const auto first_item = GetBagItem(bag_idx, start_slot_idx);
    if (!first_item)
        return true;

    if (first_item->mod_struct_size >= 10)
    {
        const auto armor_value = first_item->mod_struct[9].arg1();

        if (low_armor && armor_value >= 60U)
            return true;
        else if (!low_armor && armor_value < 60U)
            return true;
    }

    for (uint32_t offset = 0U; offset < 5U; offset++)
        EquipItemExecute(bag_idx, start_slot_idx + offset);

    return true;
}

bool LowArmor(const uint32_t bag_idx, const uint32_t start_slot_idx)
{
    return ArmorSwap(bag_idx, start_slot_idx, true);
}

bool HighArmor(const uint32_t bag_idx, const uint32_t start_slot_idx)
{
    return ArmorSwap(bag_idx, start_slot_idx, false);
}

void SortByDistance(const PlayerData &player_data, std::vector<GW::AgentLiving *> &filtered_livings)
{
    const auto player_pos = player_data.pos;

    std::sort(filtered_livings.begin(), filtered_livings.end(), [&player_pos](const auto a1, const auto a2) {
        const auto sqrd1 = GW::GetSquareDistance(player_pos, a1->pos);
        const auto sqrd2 = GW::GetSquareDistance(player_pos, a2->pos);
        return sqrd1 < sqrd2;
    });
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

void SplitFilteredAgents(const std::vector<GW::AgentLiving *> &filtered_livings,
                         std::vector<GW::AgentLiving *> &splitted_agents,
                         const uint32_t id)
{
    for (auto agent : filtered_livings)
    {
        if (agent->player_number == id)
            splitted_agents.push_back(agent);
    }
}

std::pair<GW::Agent *, float> GetClosestEnemy(const PlayerData *player_data)
{
    const auto agents = GW::Agents::GetAgentArray();
    if (!agents || !agents->valid())
        return std::make_pair(nullptr, 0.0F);

    GW::Agent *closest = nullptr;
    auto closest_dist = FLT_MAX;

    for (const auto agent : *agents)
    {
        if (!agent)
            continue;

        const auto living = agent->GetAsAgentLiving();
        if (!living)
            continue;

        if (living->allegiance != GW::Constants::Allegiance::Enemy)
            continue;

        const auto dist = GW::GetDistance(player_data->pos, living->pos);
        if (dist < closest_dist)
            closest = agent;
    }

    return std::make_pair(closest, closest_dist);
}

uint32_t GetClosesTypeID(const PlayerData &player_data, const uint32_t id, const GW::Constants::Allegiance type)
{
    std::vector<GW::AgentLiving *> agents_vec;
    FilterAgents(player_data, agents_vec, std::array<uint32_t, 1>{id}, type, GW::Constants::Range::Compass);

    if (agents_vec.size() == 0)
        return 0;

    return agents_vec[0]->agent_id;
}

uint32_t GetClosestEnemyTypeID(const PlayerData &player_data, const uint32_t id)
{
    return GetClosesTypeID(player_data, id, GW::Constants::Allegiance::Enemy);
}

uint32_t GetClosestAllyTypeID(const PlayerData &player_data, const uint32_t id)
{
    return GetClosesTypeID(player_data, id, GW::Constants::Allegiance::Ally_NonAttackable);
}

uint32_t GetClosestNpcTypeID(const PlayerData &player_data, const uint32_t id)
{
    return GetClosesTypeID(player_data, id, GW::Constants::Allegiance::Npc_Minipet);
}

uint32_t TargetClosestEnemyById(PlayerData &player_data, const uint32_t id)
{
    const auto target_id = GetClosestEnemyTypeID(player_data, id);
    if (!target_id)
        return 0;

    player_data.ChangeTarget(target_id);

    return target_id;
}

uint32_t TargetClosestAllyById(PlayerData &player_data, const uint32_t id)
{
    const auto target_id = GetClosestAllyTypeID(player_data, id);
    if (!target_id)
        return 0;

    player_data.ChangeTarget(target_id);

    return target_id;
}

uint32_t TargetClosestNpcById(PlayerData &player_data, const uint32_t id)
{
    const auto target_id = GetClosestNpcTypeID(player_data, id);
    if (!target_id)
        return 0;

    player_data.ChangeTarget(target_id);

    return target_id;
}

DWORD QuestAcceptDialog(DWORD quest)
{
    return (quest << 8) | 0x800001;
}

DWORD QuestRewardDialog(DWORD quest)
{
    return (quest << 8) | 0x800007;
}

void AttackAgent(const GW::Agent *agent)
{
    GW::CtoS::SendPacket(0xC, GAME_CMSG_ATTACK_AGENT, agent->agent_id, 0);
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
    if (!players || !players->valid())
        return false;

    party_members.clear();

    auto idx = uint32_t{0};
    for (const auto &player_data : info->players)
    {
        const auto id = (*players)[player_data.login_number].agent_id;
        party_members.push_back({id, idx});
        ++idx;

        for (const auto &hero : info->heroes)
        {
            if (hero.owner_player_id == player_data.login_number)
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

std::vector<GW::AgentLiving *> FilterAgentsByRange(const std::vector<GW::AgentLiving *> &livings,
                                                   const PlayerData &player_data,
                                                   const float dist_threshold)
{
    auto filtered_livings = std::vector<GW::AgentLiving *>{};

    for (const auto living : livings)
    {
        const auto dist = GW::GetDistance(player_data.pos, living->pos);
        if (dist <= dist_threshold)
            filtered_livings.push_back(living);
    }

    return filtered_livings;
}

std::set<uint32_t> FilterAgentIDS(const std::vector<GW::AgentLiving *> &filtered_livings,
                                  const std::set<uint32_t> &filter_ids)
{
    auto set_ids = std::set<uint32_t>{};
    auto result_ids = std::set<uint32_t>{};
    for (const auto living : filtered_livings)
    {
        set_ids.insert(static_cast<uint32_t>(living->player_number));
    }
    std::set_difference(set_ids.begin(),
                        set_ids.end(),
                        filter_ids.begin(),
                        filter_ids.end(),
                        std::inserter(result_ids, result_ids.end()));

    return result_ids;
}

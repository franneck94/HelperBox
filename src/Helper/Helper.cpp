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

const GW::EffectArray *GetEffects(const uint32_t agent_id)
{
    const auto agent_effects = GW::Effects::GetPartyEffectsArray();

    if (!agent_effects || !agent_effects->valid())
        return nullptr;

    for (const auto &agent_effect : *agent_effects)
    {
        if (agent_effect.agent_id == agent_id && agent_effect.effects.valid())
            return &agent_effect.effects;
    }

    return nullptr;
}

bool TargetNearest(const TargetType type, const float max_distance)
{
    const auto agents = GW::Agents::GetAgentArray();
    if (!agents || !agents->valid())
        return false;

    const auto me = GW::Agents::GetPlayerAsAgentLiving();
    if (!me)
        return false;

    auto distance = max_distance;
    auto closest = size_t{0};

    for (const auto agent : *agents)
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
        case TargetType::Living_Ally:
        case TargetType::Living_Enemy:
        case TargetType::Living_Npc:
        {
            const auto living_agent = agent->GetAsAgentLiving();
            if (!living_agent || !living_agent->GetIsAlive())
                continue;

            if (type == TargetType::Living_Ally &&
                living_agent->allegiance == GW::Constants::Allegiance::Ally_NonAttackable)
                break;
            else if (type == TargetType::Living_Enemy && living_agent->allegiance == GW::Constants::Allegiance::Enemy)
                break;
            else if (type == TargetType::Living_Npc &&
                     living_agent->allegiance == GW::Constants::Allegiance::Npc_Minipet)
                break;

            continue;
        }
        default:
        {
            continue;
        }
        }

        const auto newDistance = GW::GetDistance(me->pos, agent->pos);
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
    const auto effects = GW::Effects::GetPartyEffectsArray();
    if (!effects || !effects->valid())
        return false;

    const auto &buffs = (*effects)[0].buffs;
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
    if (!players || !players->valid())
        return false;

    party_members.clear();

    auto idx = uint32_t{0};
    for (const auto &player : info->players)
    {
        const auto id = (*players)[player.login_number].agent_id;
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

void SortByDistance(const Player &player, std::vector<GW::AgentLiving *> &filtered_livings)
{
    const auto player_pos = player.pos;

    std::sort(filtered_livings.begin(), filtered_livings.end(), [&player_pos](const auto a1, const auto a2) {
        const auto sqrd1 = GW::GetSquareDistance(player_pos, a1->pos);
        const auto sqrd2 = GW::GetSquareDistance(player_pos, a2->pos);
        return sqrd1 < sqrd2;
    });
}

bool CanMove()
{
    return !IsLoading() && !GW::Map::GetIsObserving();
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

    if (target_living->allegiance != GW::Constants::Allegiance::Ally_NonAttackable || target_living->GetIsDead())
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

std::pair<GW::Agent *, float> GetClosestEnemy(const Player *player)
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

        const auto dist = GW::GetDistance(player->pos, living->pos);
        if (dist < closest_dist)
            closest = agent;
    }

    return std::make_pair(closest, closest_dist);
}

uint32_t GetClosesTypeID(const Player &player, const uint32_t id, const GW::Constants::Allegiance type)
{
    std::vector<GW::AgentLiving *> agents_vec;
    FilterAgents(player, agents_vec, std::array<uint32_t, 1>{id}, type, GW::Constants::Range::Compass);

    if (agents_vec.size() == 0)
        return 0;

    return agents_vec[0]->agent_id;
}

uint32_t GetClosestEnemyTypeID(const Player &player, const uint32_t id)
{
    return GetClosesTypeID(player, id, GW::Constants::Allegiance::Enemy);
}

uint32_t GetClosestAllyTypeID(const Player &player, const uint32_t id)
{
    return GetClosesTypeID(player, id, GW::Constants::Allegiance::Ally_NonAttackable);
}

uint32_t GetClosestNpcTypeID(const Player &player, const uint32_t id)
{
    return GetClosesTypeID(player, id, GW::Constants::Allegiance::Npc_Minipet);
}

uint32_t TargetClosestEnemyById(Player &player, const uint32_t id)
{
    const auto target_id = GetClosestEnemyTypeID(player, id);
    if (!target_id)
        return 0;

    player.ChangeTarget(target_id);

    return target_id;
}

uint32_t TargetClosestAllyById(Player &player, const uint32_t id)
{
    const auto target_id = GetClosestAllyTypeID(player, id);
    if (!target_id)
        return 0;

    player.ChangeTarget(target_id);

    return target_id;
}

uint32_t TargetClosestNpcById(Player &player, const uint32_t id)
{
    const auto target_id = GetClosestNpcTypeID(player, id);
    if (!target_id)
        return 0;

    player.ChangeTarget(target_id);

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

std::vector<GW::AgentLiving *> GetEnemiesInCompass()
{
    auto living_agents = std::vector<GW::AgentLiving *>{};

    const auto agents_array = GW::Agents::GetAgentArray();
    if (!agents_array || !agents_array->valid())
        return living_agents;

    for (const auto agent : *agents_array)
    {
        if (!agent)
            continue;

        const auto living = agent->GetAsAgentLiving();
        if (!living)
            continue;

        if (living->allegiance != GW::Constants::Allegiance::Enemy)
            continue;

        if (living->GetIsDead())
            continue;

        living_agents.push_back(living);
    }

    return living_agents;
}

std::vector<GW::AgentLiving *> GetEnemiesInAggro(const Player &player)
{
    auto filtered_livings = std::vector<GW::AgentLiving *>{};

    const auto agents_array = GW::Agents::GetAgentArray();
    if (!agents_array || !agents_array->valid())
        return filtered_livings;

    for (const auto agent : *agents_array)
    {
        if (!agent)
            continue;

        const auto living = agent->GetAsAgentLiving();
        if (!living)
            continue;

        if (living->allegiance != GW::Constants::Allegiance::Enemy)
            continue;

        if (living->GetIsDead())
            continue;

        const auto dist = GW::GetDistance(player.pos, living->pos);
        if (dist <= GW::Constants::Range::Earshot)
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

void TargetAndAttackEnemyInAggro(const Player &player)
{
    if (!player.target || !player.target->agent_id || !player.target->GetIsLivingType() ||
        player.target->GetAsAgentLiving()->allegiance != GW::Constants::Allegiance::Enemy)
        TargetNearest(TargetType::Living_Enemy, GW::Constants::Range::Spellcast);

    if (player.target && player.target->agent_id)
    {
        const auto dist = GW::GetDistance(player.pos, player.target->pos);
        if (dist < GW::Constants::Range::Earshot)
            AttackAgent(player.target);
    }
}

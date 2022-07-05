#include <cstdint>

#include <GWCA/Constants/Constants.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Item.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/ItemMgr.h>
#include <GWCA/Packets/Opcodes.h>

#include <Helper.h>

#include "HelperItems.h"

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
    if (!p || p->GetIsDead() || p->GetIsKnockedDown() || p->GetIsCasting())
        return false;

    if (p->skill)
    {
        CancelMovement();
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

#pragma once

#include <cstdint>

#include <GWCA/Constants/ItemIDs.h>
#include <GWCA/GameEntities/Item.h>

bool IsWeapon(const GW::Item *item);

bool IsArmor(const GW::Item *item);

bool IsSalvagable(const GW::Item *item);

bool IsEquippable(const GW::Item *item);

bool EquipItem(const uint32_t bag_idx, const uint32_t slot_idx);

bool ArmorSwap(const uint32_t bag_idx, const uint32_t start_slot_idx, const uint32_t armor_threshold);

bool LowArmor(const uint32_t bag_idx, const uint32_t start_slot_idx);

bool HighArmor(const uint32_t bag_idx, const uint32_t start_slot_idx);

bool UseInventoryItem(const uint32_t item_id, const size_t from_bag = 1U, const size_t to_bag = 5U);

GW::WeapondSet *GetWeaponSets();

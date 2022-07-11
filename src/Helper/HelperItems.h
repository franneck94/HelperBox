#pragma once

#include <cstdint>

#include <GWCA/Constants/ItemIDs.h>
#include <GWCA/GameEntities/Item.h>

bool IsWeapon(const GW::Item *item);

bool IsMeleeWeapon(const GW::Item *item);

bool IsOffhandWeapon(const GW::Item *item);

bool IsRangeWeapon(const GW::Item *item);

bool IsArmor(const GW::Item *item);

bool IsSalvagable(const GW::Item *item);

bool IsEquippable(const GW::Item *item);

bool EquipItem(const uint32_t bag_idx, const uint32_t slot_idx);

bool ArmorSwap(const uint32_t bag_idx, const uint32_t start_slot_idx, const uint32_t armor_threshold);

bool LowArmor(const uint32_t bag_idx, const uint32_t start_slot_idx);

bool HighArmor(const uint32_t bag_idx, const uint32_t start_slot_idx);

bool UseInventoryItem(const uint32_t item_id, const size_t from_bag = 1U, const size_t to_bag = 5U);

bool WeaponSetIsMelee(const GW::WeapondSet &weapon_set);

bool WeaponSetIsRange(const GW::WeapondSet &weapon_set);

GW::WeapondSet *GetActiveWeaponSet();

GW::WeapondSet *GetWeaponSets();

std::pair<GW::WeapondSet *, uint32_t> GetFirstRangeWeaponSet();

std::pair<GW::WeapondSet *, uint32_t> GetFirstMeleeWeaponSet();

bool UseWeaponSlot(const uint32_t slot_idx);

bool SwapToMeleeSet();

bool SwapToRangeSet();

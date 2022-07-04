#pragma once

#include <cstdint>

bool EquipItemExecute(const uint32_t bag_idx, const uint32_t slot_idx);

bool ArmorSwap(const uint32_t bag_idx, const uint32_t start_slot_idx, const uint32_t armor_threshold);

bool LowArmor(const uint32_t bag_idx, const uint32_t start_slot_idx);

bool HighArmor(const uint32_t bag_idx, const uint32_t start_slot_idx);

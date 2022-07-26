#pragma once

#include <cstdint>

#include <GWCA/GameEntities/Agent.h>

bool HelperActivationConditions(const bool need_party_loaded = true);

DWORD QuestAcceptDialog(GW::Constants::QuestID quest);

DWORD QuestRewardDialog(GW::Constants::QuestID quest);

void CancelMovement();

void AttackAgent(const GW::Agent *agent);

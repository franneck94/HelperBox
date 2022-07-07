#pragma once

#include <cstdint>

#include <GWCA/GameEntities/Agent.h>

bool HelperActivationConditions();

DWORD QuestAcceptDialog(DWORD quest);

DWORD QuestRewardDialog(DWORD quest);

void CancelMovement();

void AttackAgent(const GW::Agent *agent);

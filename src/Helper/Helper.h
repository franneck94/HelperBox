#pragma once

#include <array>
#include <cstdint>
#include <set>
#include <string_view>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Skill.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/MapMgr.h>

#include <MathUtils.h>
#include <Types.h>

bool HelperActivationConditions();

DWORD QuestAcceptDialog(DWORD quest);

DWORD QuestRewardDialog(DWORD quest);

void CancelMovement();

void AttackAgent(const GW::Agent *agent);

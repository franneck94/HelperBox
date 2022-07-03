#include <functional>
#include <vector>

#include <GWCA/GameEntities/Map.h>
#include <GWCA/managers/MapMgr.h>

#include "AgentData.h"

bool AgentData::ValidateData(std::function<bool()> cb_fn) const
{
    return true;
}

void AgentData::Update()
{
}

void AgentData::UpdateAllies()
{
}

void AgentData::UpdateNeutrals()
{
}

void AgentData::UpdateEnemies()
{
}

void AgentData::UpdateSpirits()
{
}

void AgentData::UpdateMinions()
{
}

void AgentData::UpdateNpcs()
{
}

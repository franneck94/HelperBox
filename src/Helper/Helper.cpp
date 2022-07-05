#include <cstdint>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/GameContainers/Array.h>
#include <GWCA/GameEntities/Party.h>
#include <GWCA/GameEntities/Player.h>
#include <GWCA/GameEntities/Skill.h>
#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/EffectMgr.h>
#include <GWCA/Managers/MemoryMgr.h>
#include <GWCA/Managers/PartyMgr.h>

#include <Actions.h>
#include <HelperAgents.h>
#include <HelperMaps.h>
#include <PlayerData.h>

#include "Helper.h"

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

DWORD QuestAcceptDialog(DWORD quest)
{
    return (quest << 8) | 0x800001;
}

DWORD QuestRewardDialog(DWORD quest)
{
    return (quest << 8) | 0x800007;
}

void CancelMovement()
{
    GW::CtoS::SendPacket(0x4, GAME_CMSG_CANCEL_MOVEMENT);
}

void AttackAgent(const GW::Agent *agent)
{
    if (!agent)
        return;
    GW::CtoS::SendPacket(0xC, GAME_CMSG_ATTACK_AGENT, agent->agent_id, 0);
}

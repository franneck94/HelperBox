#include <GWCA/Constants/Maps.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/StoCMgr.h>
#include <GWCA/Packets/Opcodes.h>
#include <GWCA/Packets/StoC.h>
#include <GWCA/Utilities/Hook.h>

#include <Player.h>

#include "Callbacks.h"

bool SkillStoppedCallback(GW::Packet::StoC::GenericValue *packet, const Player *player)
{
    const uint32_t value_id = packet->Value_id;
    const uint32_t caster_id = packet->agent_id;

    if (caster_id != player->id)
        return false;

    if (value_id == GW::Packet::StoC::GenericValueID::skill_stopped)
        return true;

    return false;
}

bool MapLoadCallback(GW::HookStatus *status, GW::Packet::StoC::MapLoaded *packet)
{
    UNREFERENCED_PARAMETER(status);
    UNREFERENCED_PARAMETER(packet);
    switch (GW::Map::GetInstanceType())
    {
    case GW::Constants::InstanceType::Explorable:
        return true;
        break;
    case GW::Constants::InstanceType::Outpost:
    case GW::Constants::InstanceType::Loading:
    default:
        return false;
        break;
    }
}

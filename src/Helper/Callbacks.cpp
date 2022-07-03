#include <GWCA/Constants/Maps.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/StoCMgr.h>
#include <GWCA/Packets/Opcodes.h>
#include <GWCA/Packets/StoC.h>
#include <GWCA/Utilities/Hook.h>

#include <PlayerData.h>

#include "Callbacks.h"

bool ExplorableLoadCallback(GW::HookStatus *, GW::Packet::StoC::MapLoaded *)
{
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

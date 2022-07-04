#pragma once

#include <cstdint>

#include <GWCA/Managers/StoCMgr.h>
#include <GWCA/Packets/StoC.h>

bool ExplorableLoadCallback(GW::HookStatus *status, GW::Packet::StoC::MapLoaded *packet);

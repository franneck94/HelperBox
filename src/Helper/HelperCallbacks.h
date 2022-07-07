#pragma once

#include <GWCA/Managers/StoCMgr.h>
#include <GWCA/Packets/StoC.h>

#include <Types.h>

bool ExplorableLoadCallback(GW::HookStatus *status, GW::Packet::StoC::MapLoaded *packet);

bool OnChatMessageLtIsReady(GW::HookStatus *, GW::Packet::StoC::PacketBase *packet, const TriggerRole trigger_role);

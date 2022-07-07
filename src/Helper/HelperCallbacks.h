#pragma once

#include <GWCA/Managers/StoCMgr.h>
#include <GWCA/Packets/StoC.h>

bool ExplorableLoadCallback(GW::HookStatus *status, GW::Packet::StoC::MapLoaded *packet);

bool OnChatMessageLtIsReady(GW::HookStatus *, int channel, wchar_t *message);

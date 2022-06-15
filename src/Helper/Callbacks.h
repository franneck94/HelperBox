#pragma once

#include <cstdint>

#include <GWCA/Managers/StoCMgr.h>
#include <GWCA/Packets/StoC.h>

#include <Player.h>

bool SkillStoppedCallback(GW::Packet::StoC::GenericValue *packet, const Player *player);

bool ExplorableLoadCallback(GW::HookStatus *status, GW::Packet::StoC::MapLoaded *packet);

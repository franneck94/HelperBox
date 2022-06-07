#pragma once

#include <cstdint>

#include <GWCA/Managers/StoCMgr.h>
#include <GWCA/Packets/StoC.h>

#include <Player.h>

bool SkillStoppedCallback(GW::Packet::StoC::GenericValue *packet, const Player *player);

bool MapLoadCallback(GW::HookStatus *status, GW::Packet::StoC::MapLoaded *packet);

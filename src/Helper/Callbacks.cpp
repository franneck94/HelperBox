#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/StoCMgr.h>
#include <GWCA/Packets/Opcodes.h>
#include <GWCA/Packets/StoC.h>
#include <GWCA/Utilities/Hook.h>

#include <Player.h>

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

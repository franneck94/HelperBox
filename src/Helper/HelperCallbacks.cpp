#include <GWCA/Constants/Maps.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/StoCMgr.h>
#include <GWCA/Packets/Opcodes.h>
#include <GWCA/Packets/StoC.h>
#include <GWCA/Utilities/Hook.h>

#include <Logger.h>
#include <PlayerData.h>

#include "HelperCallbacks.h"

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

bool OnChatMessageLtIsReady(GW::HookStatus *, int channel, wchar_t *message)
{
    if (!message)
        return false;

    if (channel == static_cast<int>(GW::Chat::Channel::CHANNEL_GROUP))
    {
        if (std::wstring{message} == L"r")
        {
            Log::Info("LT is ready!", message);
            return true;
        }
    }

    return false;
}

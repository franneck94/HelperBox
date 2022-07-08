#pragma once

#include <array>
#include <cstdint>

#include <GWCA/GameEntities/Agent.h>
#include <GWCA/Packets/StoC.h>
#include <GWCA/Utilities/Hook.h>

#include <PlayerData.h>
#include <Types.h>

class UwHelperWindowABC
{
public:
    UwHelperWindowABC() : player_data({})
    {
        GW::StoC::RegisterPacketCallback(&SendChat_Entry,
                                         GAME_SMSG_CHAT_MESSAGE_LOCAL,
                                         [this](GW::HookStatus *status, GW::Packet::StoC::PacketBase *packet) -> void {
                                             lt_is_ready = OnChatMessageLtIsReady(status, packet, TriggerRole::LT);
                                         });

        GW::StoC::RegisterPacketCallback<GW::Packet::StoC::MapLoaded>(
            &MapLoaded_Entry,
            [this](GW::HookStatus *status, GW::Packet::StoC::MapLoaded *packet) -> void {
                load_cb_triggered = ExplorableLoadCallback(status, packet);
                num_finished_objectives = 0U;
            });

        GW::StoC::RegisterPacketCallback<GW::Packet::StoC::ObjectiveDone>(
            &ObjectiveDone_Entry,
            [this](GW::HookStatus *, GW::Packet::StoC::ObjectiveDone *packet) {
                ++num_finished_objectives;
                Log::Info("Finished Objective : %u, Num objectives: %u", packet->objective_id, num_finished_objectives);
            });

        GW::StoC::RegisterPacketCallback<GW::Packet::StoC::GenericValue>(
            &Interrupted_Entry,
            [this](GW::HookStatus *, GW::Packet::StoC::GenericValue *packet) -> void {
                if (player_data.SkillStoppedCallback(packet))
                    interrupted = true;
            });
    };

    PlayerData player_data;
    const AgentLivingData *agents_data = nullptr;

    GW::HookEntry ObjectiveDone_Entry;
    GW::HookEntry MapLoaded_Entry;
    bool load_cb_triggered = false;
    uint32_t num_finished_objectives = 0U;
    GW::HookEntry SendChat_Entry;
    bool lt_is_ready = false;
    GW::HookEntry Interrupted_Entry;
    bool interrupted = false;
};

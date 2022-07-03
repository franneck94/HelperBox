#pragma once

#include "stdafx.h"

#include <cstdint>

#include <GWCA/GameEntities/Agent.h>

#include <Actions.h>
#include <Callbacks.h>
#include <PlayerData.h>
#include <Types.h>

#include <Base/HelperBoxWindow.h>

class DhuumStatsWindow : public HelperBoxWindow
{
public:
    constexpr static auto TIME_WINDOW_DMG_S = long{180L};
    constexpr static auto TIME_WINDOW_DMG_MS = (TIME_WINDOW_DMG_S * 1000L);
    constexpr static auto TIME_WINDOW_REST_S = long{20L};
    constexpr static auto TIME_WINDOW_REST_MS = (TIME_WINDOW_REST_S * 1000L);

    constexpr static auto REST_SKILL_ID = uint32_t{3087};
    constexpr static auto REST_SKILL_REAPER_ID = uint32_t{3079U};

    constexpr static auto NEEDED_NUM_REST = std::array<uint32_t, 8>{uint32_t{780U},  // 1 ?
                                                                    uint32_t{760U},  // 2 ?
                                                                    uint32_t{740U},  // 3 ?
                                                                    uint32_t{720U},  // 4
                                                                    uint32_t{700U},  // 5
                                                                    uint32_t{680U},  // 6 ?
                                                                    uint32_t{660U},  // 7 ?
                                                                    uint32_t{640U}}; // 8 ?

private:
    void SkillPacketCallback(const uint32_t value_id,
                             const uint32_t caster_id,
                             const uint32_t target_id,
                             const uint32_t value,
                             const bool no_target);
    void DamagePacketCallback(const uint32_t type,
                              const uint32_t caster_id,
                              const uint32_t target_id,
                              const float value);

    void ResetData();
    void RemoveOldData();
    void UpdateRestData();
    void UpdateDamageData();

public:
    DhuumStatsWindow() : player_data({}), rests({}), damages({})
    {
        /* Skill on self or party player_data */
        GW::StoC::RegisterPacketCallback<GW::Packet::StoC::GenericValue>(
            &SkillCasted_Entry,
            [this](GW::HookStatus *, GW::Packet::StoC::GenericValue *packet) -> void {
                const uint32_t value_id = packet->Value_id;
                const uint32_t caster_id = packet->agent_id;
                const uint32_t target_id = 0U;
                const uint32_t value = packet->value;
                const bool no_target = true;
                SkillPacketCallback(value_id, caster_id, target_id, value, no_target);
            });

        GW::StoC::RegisterPacketCallback<GW::Packet::StoC::GenericModifier>(
            &Damage_Entry,
            [this](GW::HookStatus *, GW::Packet::StoC::GenericModifier *packet) -> void {
                const uint32_t type = packet->type;
                const uint32_t caster_id = packet->cause_id;
                const uint32_t target_id = packet->target_id;
                const float value = packet->value;
                DamagePacketCallback(type, caster_id, target_id, value);
            });
    };
    ~DhuumStatsWindow(){};

    static DhuumStatsWindow &Instance()
    {
        static DhuumStatsWindow instance;
        return instance;
    }

    const char *Name() const override
    {
        return "DhuumStatsWindow";
    }

    void Draw(IDirect3DDevice9 *pDevice) override;
    void Update(float delta, const PlayerData &, const AgentLivingData &) override;

private:
    PlayerData player_data;

    GW::HookEntry SkillCasted_Entry;
    GW::HookEntry Damage_Entry;

    uint32_t dhuum_id = 0U;
    float dhuum_hp = 0.0F;
    uint32_t dhuum_max_hp = 0U;

    long dhuum_fight_start_time_ms = 0L;
    bool dhuum_fight_active = false;

    uint32_t num_casted_rest = 0U;
    float rests_per_s = 0.0F;
    float eta_rest = 0.0F;
    std::vector<long> rests;

    uint32_t num_attacks = 0U;
    float damage_per_s = 0.0F;
    float eta_damage = 0.0F;
    std::vector<std::pair<long, float>> damages;
};

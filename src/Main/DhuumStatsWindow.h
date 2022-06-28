#pragma once

#include "stdafx.h"

#include <cstdint>

#include <GWCA/GameEntities/Agent.h>

#include <Actions.h>
#include <Callbacks.h>
#include <Player.h>
#include <Types.h>

#include <Base/HelperBoxWindow.h>

class DhuumStatsWindow : public HelperBoxWindow
{
public:
    constexpr static auto REST_SKILL_ID = uint32_t{3087};
    constexpr static auto REST_SKILL_REAPER_ID = uint32_t{3079U};
    constexpr static auto NEEDED_NUM_REST = uint32_t{741U};

private:
    void SkillPacketCallback(const uint32_t value_id,
                             const uint32_t caster_id,
                             const uint32_t target_id,
                             const uint32_t value,
                             const bool no_target)
    {
        uint32_t agent_id = caster_id;
        const uint32_t activated_skill_id = value;

        // ignore non-skill packets
        switch (value_id)
        {
        case GW::Packet::StoC::GenericValueID::instant_skill_activated:
        case GW::Packet::StoC::GenericValueID::skill_activated:
        case GW::Packet::StoC::GenericValueID::skill_finished:
        case GW::Packet::StoC::GenericValueID::attack_skill_activated:
        case GW::Packet::StoC::GenericValueID::attack_skill_finished:
        {
            if (!no_target)
                agent_id = target_id;
            break;
        }
        default:
            return;
        }

        if (REST_SKILL_ID == activated_skill_id || REST_SKILL_REAPER_ID == activated_skill_id)
        {
            ++num_casted_rest;
            rests.push_back(clock());
        }
    }

    void DamagePacketCallback(const uint32_t type,
                              const uint32_t caster_id,
                              const uint32_t target_id,
                              const float value)
    {
        if (!dhuum_id || target_id != dhuum_id)
            return;

        // ignore non-damage packets
        switch (type)
        {
        case GW::Packet::StoC::P156_Type::damage:
        case GW::Packet::StoC::P156_Type::critical:
        case GW::Packet::StoC::P156_Type::armorignoring:
            break;
        default:
            return;
        }

        if (value >= 0 || !caster_id)
            return;

        const auto caster_agent = GW::Agents::GetAgentByID(caster_id);
        if (!caster_agent)
            return;

        const auto caster_living = caster_agent->GetAsAgentLiving();
        if (!caster_living)
            return;

        if (caster_living->allegiance != static_cast<uint8_t>(GW::Constants::Allegiance::Ally_NonAttackable) &&
            caster_living->allegiance != static_cast<uint8_t>(GW::Constants::Allegiance::Npc_Minipet))
            return;

        const auto target_agent = GW::Agents::GetAgentByID(target_id);
        if (!target_agent)
            return;

        const auto target_living = target_agent->GetAsAgentLiving();
        if (!target_living)
            return;

        const auto dmg_f32 = -value * target_living->max_hp;
        ++num_attacks;

        const auto time = clock();
        damages.push_back(std::make_pair(time, dmg_f32));
    }

    void ResetData();
    void RemoveOldData();
    void UpdateRestData();
    void UpdateDamageData();

public:
    DhuumStatsWindow() : player({}), rests({}), damages({})
    {
        /* Skill on self or party player */
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

    void Initialize() override
    {
        HelperBoxWindow::Initialize();
    }

    void LoadSettings(CSimpleIni *ini) override
    {
        HelperBoxWindow::LoadSettings(ini);
        show_menubutton = true;
    }

    void SaveSettings(CSimpleIni *ini) override
    {
        HelperBoxWindow::SaveSettings(ini);
    }

    void Draw(IDirect3DDevice9 *pDevice) override;
    void Update(float delta) override;

private:
    Player player;

    GW::HookEntry SkillCasted_Entry;
    GW::HookEntry Damage_Entry;

    const long TIME_WINDOW_S = 120L;
    const long TIME_WINDOW_MS = (TIME_WINDOW_S * 1000L);

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

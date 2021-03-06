#include <array>
#include <cmath>
#include <cstdint>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Context/GameContext.h>
#include <GWCA/Context/WorldContext.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Party.h>
#include <GWCA/GameEntities/Player.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Managers/UIMgr.h>

#include <ActionTypes.h>
#include <ActionsBase.h>
#include <Base/HelperBox.h>
#include <DataPlayer.h>
#include <Helper.h>
#include <HelperMaps.h>
#include <HelperPackets.h>

#include <imgui.h>

#include "HeroWindow.h"

uint32_t HeroWindow::GetNumPlayerHeroes()
{
    auto num = uint32_t{0};
    for (const auto &hero : *party_heros)
    {
        if (hero.owner_player_id == player_data.living->login_number)
            ++num;
    }
    return num;
}

void HeroWindow::ToggleHeroBehaviour()
{
    if (!IsMapReady() || !party_heros)
        return;

    Log::Info("Toggle hero hehaviour!");

    if (current_hero_behaviour == HeroBehaviour::GUARD)
        current_hero_behaviour = HeroBehaviour::AVOID_COMBAT;
    else if (current_hero_behaviour == HeroBehaviour::ATTACK)
        current_hero_behaviour = HeroBehaviour::GUARD;
    else if (current_hero_behaviour == HeroBehaviour::AVOID_COMBAT)
        current_hero_behaviour = HeroBehaviour::ATTACK;

    for (const auto &hero : *party_heros)
    {
        if (hero.owner_player_id == player_data.living->login_number)
            GW::CtoS::SendPacket(0xC, GAME_CMSG_HERO_BEHAVIOR, hero.agent_id, (uint32_t)current_hero_behaviour);
    }
}

void HeroWindow::FollowPlayer()
{
    if (!IsMapReady())
        return;

    GW::PartyMgr::FlagAll(follow_pos);
}

void HeroWindow::AttackTarget()
{
    if (!IsMapReady() || !target_agent_id || !party_heros)
        return;

    const auto target_agent = GW::Agents::GetAgentByID(target_agent_id);
    if (!target_agent)
        return;
    const auto target_living = target_agent->GetAsAgentLiving();
    if (!target_living)
        return;

    if (target_living->allegiance != GW::Constants::Allegiance::Enemy)
        return;

    Log::Info("Heroes will attack the players target!");

    for (const auto &hero : *party_heros)
    {
        if (hero.owner_player_id == player_data.living->login_number)
        {
            GW::CtoS::SendPacket(0xC, GAME_CMSG_HERO_LOCK_TARGET, hero.agent_id, target_agent_id);

            const auto hero_agent = GW::Agents::GetAgentByID(hero.agent_id);
            if (!hero_agent)
                continue;
            const auto hero_living = hero_agent->GetAsAgentLiving();
            if (!hero_living)
                continue;

            if (hero_living->primary == static_cast<uint8_t>(GW::Constants::Profession::Mesmer))
            {
                // const auto skillbar_array = GW::SkillbarMgr::GetSkillbarArray();
                // if (!skillbar_array)
                //     continue;

                // auto hero_has_spike_skill = false;
                // for (const auto &skillbar : *skillbar_array)
                // {
                //     if (skillbar.agent_id != hero_living->agent_id)
                //         continue;

                //     if (skillbar.skills[0].skill_id != (uint32_t)GW::Constants::SkillID::Energy_Surge)
                //         continue;

                //     hero_has_spike_skill = true;
                // }
                // if (!hero_has_spike_skill)
                //     return;

                // GW::SkillbarMgr::HeroUseSkill(hero.agent_id, 1, target_agent_id, 0);
                // GW::Packet::HeroUseSkill packet = {.header = GAME_CMSG_HERO_USE_SKILL,
                //                                    .hero_id = hero.agent_id,
                //                                    .target_id = target_agent_id,
                //                                    .skill_slot = 1};
                // GW::CtoS::SendPacket(&packet);

                // GW::CtoS::SendPacket(16, GAME_CMSG_HERO_USE_SKILL, hero.hero_id, target_agent_id, 1);

                // auto action = GW::UI::ControlAction{};
                // action = GW::UI::ControlAction::ControlAction_Hero1Skill1;

                // GW::GameThread::Enqueue([&]() { GW::UI::Keypress(action); });
            }
        }
    }
}

void HeroWindow::Draw()
{
    static auto last_follow_trigger_ms = clock();

    if (!visible)
        return;

    if (!player_data.ValidateData(HelperActivationConditions, false))
        return;

    if (!party_heros || party_heros->size() == 0 || GetNumPlayerHeroes() == 0)
        return;

    ImGui::SetNextWindowSize(ImVec2(240.0F, 45.0F), ImGuiCond_FirstUseEver);

    if (ImGui::Begin(Name(), nullptr, GetWinFlags() | ImGuiWindowFlags_NoDecoration))
    {
        const auto width = ImGui::GetWindowWidth();
        auto added_color_follow = false;
        auto toggled_follow = false;

        switch (current_hero_behaviour)
        {
        case HeroBehaviour::GUARD:
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1F, 0.1F, 1.0F, 1.0F));
            break;
        }
        case HeroBehaviour::AVOID_COMBAT:
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1F, 0.9F, 0.1F, 1.0F));
            break;
        }
        case HeroBehaviour::ATTACK:
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0F, 0.1F, 0.1F, 1.0F));
            break;
        }
        }
        if (ImGui::Button("Behaviour###toggleState", ImVec2{width / 3.0F - 10.0F, 50.0F}))
        {
            ToggleHeroBehaviour();
        }
        ImGui::PopStyleColor();
        if (following_active)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1F, 0.9F, 0.1F, 1.0F));
            added_color_follow = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Follow###followPlayer", ImVec2{width / 3.0F - 10.0F, 50.0F}))
        {
            following_active = !following_active;
            toggled_follow = true;

            Log::Info("Heroes will follow the player!");
        }
        if (following_active && TIMER_DIFF(last_follow_trigger_ms) > 800)
        {
            FollowPlayer();
            last_follow_trigger_ms = clock();
        }
        else if (toggled_follow)
        {
            GW::PartyMgr::UnflagAll();
        }
        if (added_color_follow)
            ImGui::PopStyleColor();
        ImGui::SameLine();
        if (ImGui::Button("Attack###attackTarget", ImVec2{width / 3.0F - 10.0F, 50.0F}))
        {
            AttackTarget();
        }
    }
    ImGui::End();
}

void HeroWindow::Update(float, const AgentLivingData &)
{
    if (!player_data.ValidateData(HelperActivationConditions, false))
        return;
    player_data.Update();

    const auto *const party_info = GW::PartyMgr::GetPartyInfo();
    if (party_info)
        party_heros = &party_info->heroes;

    follow_pos = player_data.pos;
    if (player_data.target)
        target_agent_id = player_data.target->agent_id;
    else
        target_agent_id = 0U;
}

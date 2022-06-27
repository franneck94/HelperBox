#include "stdafx.h"

#include <algorithm>
#include <array>
#include <cstdint>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Player.h>

#include <HelperBox.h>

#include <Actions.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <MathUtils.h>
#include <Player.h>
#include <Types.h>
#include <UwHelper.h>

#include "DhuumStatsWindow.h"

void DhuumStatsWindow::Draw(IDirect3DDevice9 *pDevice)
{
    UNREFERENCED_PARAMETER(pDevice);

    if (!visible)
        return;

    if (!player.ValidateData(UwHelperActivationConditions))
        return;

    ImGui::SetNextWindowSize(ImVec2(170.0F, 175.0F), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("DhuumStatsWindow", nullptr, GetWinFlags() | ImGuiWindowFlags_NoScrollbar))
    {
        const auto width = ImGui::GetWindowWidth();

        ImGui::Text("Dhuum HP: %3.0f%%", dhuum_hp * 100.0F);
        const auto timer_ms = TIMER_DIFF(dhuum_fight_start_time_ms);
        ImGui::Text("Timer: %4.0f", static_cast<float>(timer_ms) / 1000.0F);
        ImGui::Separator();
        ImGui::Text("Rests: %u", num_casted_rest);
        ImGui::Text("Rests (per s): %0.2f", rests_per_s);
        ImGui::Text("ETA Rest (s): %0.2f", (num_casted_rest > 0 && eta_rest < 10'000.0F) ? eta_rest : 0.0F);
        ImGui::Separator();
        ImGui::Text("Num Attacks: %u", num_attacks);
        ImGui::Text("Damage (per s): %0.2f", damage_per_s);
        ImGui::Text("ETA Damage (s): %0.2f", (num_attacks > 0 && eta_damage < 10'000.0F) ? eta_damage : 0.0F);
    }
    ImGui::End();
}

void DhuumStatsWindow::ResetData()
{
    dhuum_fight_active = true;
    dhuum_fight_start_time_ms = clock();

    num_casted_rest = 0U;
    rests_per_s = 0.0F;
    eta_rest = 0.0F;
    rests.clear();

    num_attacks = 0U;
    damage_per_s = 0.0F;
    eta_damage = 0.0F;
    damages.clear();
}

void DhuumStatsWindow::RemoveOldData()
{
    const auto remove_rest_it = std::remove_if(rests.begin(), rests.end(), [=](const auto t) {
        return std::abs(TIMER_DIFF(t)) > TIME_WINDOW_MS;
    });
    rests.erase(remove_rest_it, rests.end());

    const auto remove_dmg_it = std::remove_if(damages.begin(), damages.end(), [=](const auto p) {
        return std::abs(TIMER_DIFF(p.first)) > TIME_WINDOW_MS;
    });
    damages.erase(remove_dmg_it, damages.end());
}

void DhuumStatsWindow::UpdateRestData()
{
    const auto current_time_ms = clock();

    auto idx_rests = uint32_t{0};
    for (const auto time : rests)
    {
        if (time >= current_time_ms - TIME_WINDOW_MS)
            ++idx_rests;
    }

    if (idx_rests > 0)
        rests_per_s = static_cast<float>(idx_rests) / static_cast<float>(TIME_WINDOW_S);
    else
        rests_per_s = 0.0F;

    const auto still_needed_rest = NEEDED_NUM_REST - num_casted_rest;
    eta_rest = still_needed_rest / (rests_per_s + FLT_EPSILON);
}

void DhuumStatsWindow::UpdateDamageData()
{
    const auto current_time_ms = clock();

    auto idx_dmg = uint32_t{0};
    for (const auto [time, dmg] : damages)
    {
        if (time >= current_time_ms - TIME_WINDOW_MS)
        {
            damage_per_s += dmg;
            ++idx_dmg;
        }
    }

    if (idx_dmg > 0)
        damage_per_s /= static_cast<float>(TIME_WINDOW_S);
    else
        damage_per_s = 0.0F;

    if (dhuum_hp >= 0.25F && dhuum_hp < 1.0F && damage_per_s > 0.0F)
        eta_damage = ((dhuum_max_hp * dhuum_hp) - (dhuum_max_hp * 0.25)) / damage_per_s;
    else
        eta_damage = 0.0F;
}

void DhuumStatsWindow::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);

    if (!player.ValidateData(UwHelperActivationConditions))
        return;
    player.Update();

    const auto is_in_dhuum_room = IsInDhuumRoom(player);
    if (!is_in_dhuum_room)
        ResetData();

    const auto is_in_dhuum_fight = IsInDhuumFight(&dhuum_id, &dhuum_hp, &dhuum_max_hp);
    if (!is_in_dhuum_fight || !dhuum_id)
        ResetData();

    if (dhuum_hp == 1.0F)
        ResetData();

    RemoveOldData();
    UpdateRestData();
    UpdateDamageData();
}

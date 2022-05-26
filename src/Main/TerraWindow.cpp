#include "stdafx.h"

#include <array>
#include <cstdint>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/Context/GameContext.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Player.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/EffectMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/PlayerMgr.h>
#include <GWCA/Managers/SkillbarMgr.h>
#include <GWCA/Packets/Opcodes.h>

#include <fmt/format.h>

#include <HelperBox.h>
#include <Logger.h>
#include <Timer.h>

#include <Actions.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <Player.h>
#include <Skillbars.h>
#include <Types.h>
#include <Utils.h>

#include "TerraWindow.h"

namespace
{
static const auto DEFAULT_WINDOW_SIZE = ImVec2(100.0F, 100.0F);
static constexpr auto HEALING_SPRING_U16 = static_cast<uint16_t>(GW::Constants::SkillID::Healing_Spring);
static constexpr auto MIN_IDLE_TIME_S = 0.1F;
static constexpr auto MAX_TABLE_LENGTH = 6U;
} // namespace

void TerraWindow::Draw(IDirect3DDevice9 *pDevice)
{
    UNREFERENCED_PARAMETER(pDevice);

    if (IsLoading())
        return;

    if (!visible)
        return;

    if (IsExplorable() && GW::Map::GetMapID() != GW::Constants::MapID::The_Underworld)
        return;

    ImGui::SetNextWindowSize(DEFAULT_WINDOW_SIZE, ImGuiCond_FirstUseEver);

    if (ImGui::Begin("TerraWindow", nullptr, GetWinFlags()))
    {
        uint32_t idx = 0;
        for (const auto &foe : filtered_foes)
        {
            bool pushed = false;
            if (foe->GetIsDead())
                continue;

            if (foe->GetIsCasting() && foe->skill == HEALING_SPRING_U16)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1F, 0.9F, 0.1F, 1.0));
                pushed = true;
            }
            const float distance = GW::GetDistance(player.pos, foe->pos);
            ImGui::Text("B: %3.0f, %4.0f", foe->hp * 100.0F, distance);
            if (pushed)
            {
                ImGui::PopStyleColor();
            }

            ImGui::SameLine();
            const auto label = fmt::format("Target##{}", idx);
            if (ImGui::Button(label.data()))
            {
                player.ChangeTarget(foe->agent_id);
            }

            ++idx;

            if (idx >= MAX_TABLE_LENGTH)
            {
                break;
            }
        }
    }

    ImGui::End();
}

void TerraWindow::Update(float delta)
{
    static float idle_time_ms = 0;

    if (!player.ValidateData())
        return;

    player.Update();

    filtered_foes.clear();
    auto agents_array = GW::Agents::GetAgentArray();
    FilterAgents(player,
                 agents_array,
                 filtered_foes,
                 GW::Constants::ModelID::UW::ObsidianBehemoth,
                 GW::Constants::Range::Spellcast);
    SortByDistance(player, filtered_foes);

    if (!player.living->GetIsMoving())
    {
        idle_time_ms += delta;
    }
    else
    {
        idle_time_ms = 0.0F;
    }

    if (filtered_foes.size() == 0)
        return;

    const auto nearest_behemoth = filtered_foes[0];
    const auto dist_nearest = GW::GetDistance(player.pos, nearest_behemoth->pos);

    if (idle_time_ms > MIN_IDLE_TIME_S && !player.living->GetIsMoving() && dist_nearest < GW::Constants::Range::Area)
    {
        for (const auto &foe : filtered_foes)
        {
            if (!foe)
                continue;

            const auto dist = GW::GetDistance(player.pos, foe->pos);

            if (dist < GW::Constants::Range::Earshot && foe->GetIsCasting() && foe->skill == HEALING_SPRING_U16)
            {
                player.ChangeTarget(foe->agent_id);
            }
        }
    }
}

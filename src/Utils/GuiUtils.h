#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Camera.h>
#include <GWCA/Managers/CameraMgr.h>

#include <Actions.h>
#include <GuiConstants.h>
#include <Helper.h>
#include <MathUtils.h>
#include <Move.h>
#include <Types.h>

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

void DrawButton(ActionState &action_state,
                const ImVec4 color,
                std::string_view text,
                const ImVec2 button_size = DEFAULT_BUTTON_SIZE);

template <uint32_t N>
void DrawMovingButtons(const std::array<MoveABC *, N> &moves, bool &move_ongoing, uint32_t &move_idx)
{
    bool was_already_ongoing = move_ongoing;
    if (was_already_ongoing)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1F, 0.9F, 0.1F, 1.0F));
    if (ImGui::Button(moves[move_idx]->Name(), DEFAULT_BUTTON_SIZE))
    {
        if (!move_ongoing)
        {
            if (!moves[move_idx]->is_distance_based)
                moves[move_idx]->Execute();
            move_ongoing = true;
        }
        else
        {
            move_ongoing = false;
            CancelMovement();
        }
    }
    if (was_already_ongoing)
        ImGui::PopStyleColor();

    if (ImGui::Button("Prev.", SKIP_BUTTON_SIZE))
    {
        if (move_idx > 0)
            --move_idx;
        else
            move_idx = moves.size() - 1;
        move_ongoing = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Next", SKIP_BUTTON_SIZE))
    {
        if (move_idx < moves.size() - 1)
            ++move_idx;
        else
            move_idx = 0;
        move_ongoing = false;
    }
}

void PlotRectangleLine(const GW::GamePos &player_pos,
                       const GW::GamePos &p1,
                       const GW::GamePos &p2,
                       std::string_view label);

void PlotPoint(const GW::GamePos &player_pos,
               const GW::GamePos &p,
               std::string_view label,
               const ImVec4 &color,
               const float width = 5.0F);

void PlotCircle(const GW::GamePos &player_pos, std::string_view label, const ImVec4 &color);

void PlotEnemies(const GW::GamePos &player_pos,
                 const std::vector<GW::AgentLiving *> &living_agents,
                 std::string_view label,
                 const ImVec4 &color);

void DrawMap(const GW::GamePos &player_pos,
             const std::vector<GW::AgentLiving *> &enemies,
             const GW::GamePos &move_pos,
             std::string_view label);

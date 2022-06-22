#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include <Actions.h>
#include <Types.h>

#include <GuiConstants.h>

#include <imgui.h>
#include <implot.h>

void DrawButton(ActionState &action_state,
                const ImVec4 color,
                std::string_view text,
                const ImVec2 button_size = DEFAULT_BUTTON_SIZE);

template <typename T, uint32_t N>
void DrawMovingButtons(const std::array<T, N> &moves, bool &move_state_active, uint32_t &move_idx)
{
    if (ImGui::Button(moves[move_idx].Name(), DEFAULT_BUTTON_SIZE))
    {
        moves[move_idx].Execute();
        move_state_active = true;
    }
    if (ImGui::Button("Prev.", SKIP_BUTTON_SIZE))
    {
        if (move_idx > 0)
            --move_idx;
        else
            move_idx = moves.size() - 1;
        move_state_active = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Next", SKIP_BUTTON_SIZE))
    {
        if (move_idx < moves.size() - 1)
            ++move_idx;
        else
            move_idx = 0;
        move_state_active = false;
    }
}

void plot_shaded_rect(const float x1,
                      const float x2,
                      const float y1,
                      const float y2,
                      const float y3,
                      const float y4,
                      std::string_view label);

void plot_rectangle_line(const GW::GamePos &p1, const GW::GamePos &p2, std::string_view label);

void plot_point(const GW::GamePos &p, std::string_view label, const ImVec4 &color, const float width = 5.0F);

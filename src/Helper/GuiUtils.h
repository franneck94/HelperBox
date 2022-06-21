#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include <imgui.h>

#include <Actions.h>
#include <Types.h>

#include <GuiConstants.h>

void DrawButton(ActionState &action_state,
                const ImVec4 color,
                std::string_view text,
                const ImVec2 button_size = DEFAULT_BUTTON_SIZE);

template <typename T, uint32_t N>
void DrawMovingButtons(const std::array<T, N> &moves, bool &send_move, uint32_t &move_idx)
{
    if (ImGui::Button(moves[move_idx].Name(), DEFAULT_BUTTON_SIZE))
    {
        moves[move_idx].Execute();
        send_move = true;
    }
    if (ImGui::Button("Prev.", SKIP_BUTTON_SIZE))
    {
        if (move_idx > 0)
            --move_idx;
        else
            move_idx = moves.size() - 1;
        send_move = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Next", SKIP_BUTTON_SIZE))
    {
        if (move_idx < moves.size() - 1)
            ++move_idx;
        else
            move_idx = 0;
        send_move = false;
    }
}

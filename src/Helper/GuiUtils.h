#pragma once

#include <cstdint>
#include <string_view>

#include <imgui.h>

#include <Types.h>

static const auto DEFAULT_BUTTON_SIZE = ImVec2(100.0, 50.0);

void DrawButton(ActionState &action_state,
                const ImVec4 color,
                std::string_view text,
                const ImVec2 button_size = DEFAULT_BUTTON_SIZE);

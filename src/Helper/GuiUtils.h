#pragma once

#include <string_view>

#include <imgui.h>

#include <Actions.h>

static const auto WINDOW_SIZE = ImVec2(120.0, 370.0);
static const auto BUTTON_SIZE = ImVec2(120.0, 80.0);

void DrawButton(ActionState &action_state, const ImVec4 color, std::string_view text);

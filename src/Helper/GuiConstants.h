#pragma once

#include <Types.h>

#include <imgui.h>

static const auto DEFAULT_BUTTON_SIZE = ImVec2(100.0, 50.0);

static const auto SKIP_BUTTON_X = DEFAULT_BUTTON_SIZE.x / 2.25F;
static const auto SKIP_BUTTON_Y = DEFAULT_BUTTON_SIZE.y / 2.0F;
static const auto SKIP_BUTTON_SIZE = ImVec2(SKIP_BUTTON_X, SKIP_BUTTON_Y);

static const auto ACTIVE_COLOR = ImVec4{0.0F, 200.0F, 0.0F, 80.0F};
static const auto INACTIVE_COLOR = ImVec4{41.0F, 74.0F, 122.0F, 80.0F};
static const auto ON_HOLD_COLOR = ImVec4{255.0F, 226.0F, 0.0F, 80.0F};

static auto COLOR_MAPPING = std::map<uint32_t, ImVec4>{{static_cast<uint32_t>(ActionState::INACTIVE), INACTIVE_COLOR},
                                                       {static_cast<uint32_t>(ActionState::ACTIVE), ACTIVE_COLOR},
                                                       {static_cast<uint32_t>(ActionState::ON_HOLD), ON_HOLD_COLOR}};

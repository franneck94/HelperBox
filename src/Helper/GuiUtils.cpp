#include <string_view>

#include <Helper.h>
#include <Types.h>

#include "GuiUtils.h"

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

void DrawButton(ActionState &action_state, const ImVec4 color, std::string_view text, const ImVec2 button_size)
{
    auto pushed_style = false;

    if (action_state != ActionState::INACTIVE)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        pushed_style = true;
    }

    if (ImGui::Button(text.data(), button_size))
    {
        if (IsExplorable())
            action_state = StateNegation(action_state);
    }
    if (pushed_style)
        ImGui::PopStyleColor();
}

void plot_shaded_rect(const float x1,
                      const float x2,
                      const float y1,
                      const float y2,
                      const float y3,
                      const float y4,
                      std::string_view label)
{
    const auto num_points = size_t{2};

    const auto xs = std::array<float, num_points>{x1, x2};
    const auto ys1 = std::array<float, num_points>{y1, y2};
    const auto ys2 = std::array<float, num_points>{y3, y4};

    ImPlot::PlotShaded(label.data(), xs.data(), ys1.data(), ys2.data(), num_points);
}

void plot_rectangle_line(const GW::GamePos &p1, const GW::GamePos &p2, std::string_view label)
{
    const float xs[2] = {p1.x, p2.x};
    const float ys[2] = {p1.y, p2.y};
    ImPlot::SetNextLineStyle(ImVec4{1.0F, 0.7F, 0.1F, 1.0F}, 2.0F);
    ImPlot::PlotLine(label.data(), xs, ys, 2);
}

void plot_point(const GW::GamePos &p, std::string_view label, const ImVec4 &color, const float width)
{
    const float xs[1] = {p.x};
    const float ys[1] = {p.y};
    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, width, color, 1.0F, color);
    ImPlot::PlotScatter(label.data(), xs, ys, 1);
}

void plot_circle(const Player &player, std::string_view label, const ImVec4 &color)
{
    for (int i = 0; i < 360; i++)
    {
        const auto label_ = fmt::format("{}###{}", label.data(), i);
        const auto x_p = player.pos.x + 1050.0F * std::sin((float)i);
        const auto y_p = player.pos.y + 1050.0F * std::cos((float)i);
        const auto pos = GW::GamePos{x_p, y_p, 0};
        plot_point(pos, label_, color, 1.0F);
    }
}

void plot_enemies(const std::vector<GW::AgentLiving *> &living_agents, std::string_view label, const ImVec4 &color)
{
    auto idx = 0U;
    for (const auto living : living_agents)
    {
        if (!living)
            continue;
        const auto label_ = fmt::format("{}##{}", label.data(), idx);
        plot_point(living->pos, label_, color);
        ++idx;
    }
}

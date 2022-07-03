#include <string_view>

#include <Helper.h>

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

void PlotRectangleLine(const GW::GamePos &player_pos,
                       const GW::GamePos &p1,
                       const GW::GamePos &p2,
                       std::string_view label)
{
    auto cam = GW::CameraMgr::GetCamera();
    if (!cam)
        return;
    const auto angle = (cam->GetCurrentYaw() + static_cast<float>(M_PI_2));
    const auto p1_ = RotatePoint(player_pos, p1, angle);
    const auto p2_ = RotatePoint(player_pos, p2, angle);

    const float xs[2] = {p1_.x * -1.0F, p2_.x * -1.0F};
    const float ys[2] = {p1_.y, p2_.y};
    ImPlot::SetNextLineStyle(ImVec4{1.0F, 0.7F, 0.1F, 1.0F}, 2.0F);
    ImPlot::PlotLine(label.data(), xs, ys, 2);
}

void PlotPoint(const GW::GamePos &player_pos,
               const GW::GamePos &p,
               std::string_view label,
               const ImVec4 &color,
               const float width)
{
    auto cam = GW::CameraMgr::GetCamera();
    if (!cam)
        return;
    const auto angle = (cam->GetCurrentYaw() + static_cast<float>(M_PI_2));
    const auto p_ = RotatePoint(player_pos, p, angle);

    const float xs[1] = {p_.x * -1.0F};
    const float ys[1] = {p_.y};
    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, width, color, 1.0F, color);
    ImPlot::PlotScatter(label.data(), xs, ys, 1);
}

void PlotCircle(const GW::GamePos &player_pos, std::string_view label, const ImVec4 &color)
{
    for (int i = 0; i < 360; i++)
    {
        const auto label_ = fmt::format("{}###{}", label.data(), i);
        const auto x_p = player_pos.x + 1050.0F * std::sin((float)i);
        const auto y_p = player_pos.y + 1050.0F * std::cos((float)i);
        const auto pos = GW::GamePos{x_p, y_p, 0};
        PlotPoint(player_pos, pos, label_, color, 1.0F);
    }
}

void PlotEnemies(const GW::GamePos &player_pos,
                 const std::vector<GW::AgentLiving *> &living_agents,
                 std::string_view label,
                 const ImVec4 &color)
{
    auto idx = 0U;
    for (const auto living : living_agents)
    {
        if (!living)
            continue;
        const auto label_ = fmt::format("{}##{}", label.data(), idx);
        if (living->login_number == GW::Constants::ModelID::UW::SkeletonOfDhuum1 ||
            living->login_number == GW::Constants::ModelID::UW::SkeletonOfDhuum2)
            PlotPoint(player_pos, living->pos, label_, ImVec4{0.0F, 0.0F, 1.0f, 1.0F});
        else
            PlotPoint(player_pos, living->pos, label_, color);
        ++idx;
    }
}

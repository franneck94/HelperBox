#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Camera.h>
#include <GWCA/Managers/CameraMgr.h>

#include <Actions.h>
#include <Types.h>

#include <GuiConstants.h>
#include <Helper.h>
#include <MathUtils.h>
#include <Move.h>

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

void DrawButton(ActionState &action_state,
                const ImVec4 color,
                std::string_view text,
                const ImVec2 button_size = DEFAULT_BUTTON_SIZE);

template <typename T, uint32_t N>
void DrawMovingButtons(const std::array<T, N> &moves, bool &move_ongoing, uint32_t &move_idx)
{
    bool was_already_ongoing = move_ongoing;
    if (was_already_ongoing)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1F, 0.9F, 0.1F, 1.0));
    if (ImGui::Button(moves[move_idx].Name(), DEFAULT_BUTTON_SIZE))
    {
        if (!move_ongoing)
        {
            moves[move_idx].Execute();
            move_ongoing = true;
        }
        else
        {
            move_ongoing = false;
            GW::CtoS::SendPacket(0x4, GAME_CMSG_CANCEL_MOVEMENT);
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

template <typename T, uint32_t N>
void DrawMap(const GW::GamePos &player_pos,
             const std::vector<GW::AgentLiving *> &enemies,
             const std::array<T, N> &moves,
             const uint32_t move_idx,
             std::string_view label)
{
    const auto cam = GW::CameraMgr::GetCamera();
    if (!cam)
        return;
    const auto theta = cam->GetCurrentYaw() - static_cast<float>(M_PI_2);
    if (std::isnan(theta))
        return;

    ImGui::SetNextWindowSize(ImVec2{450.0F, 450.0F}, ImGuiCond_FirstUseEver);
    const auto label_window = fmt::format("{}Window", label.data());
    if (ImGui::Begin(label_window.data(), nullptr, ImGuiWindowFlags_None))
    {
        const auto label_plot = fmt::format("{}Plot", label.data());
        if (ImPlot::BeginPlot(label_plot.data(), ImVec2{400.0F, 400.0F}, ImPlotFlags_CanvasOnly))
        {
            const auto next_pos = moves[move_idx].pos;
            const auto rect = GameRectangle(player_pos, next_pos, GW::Constants::Range::Spellcast);

            const auto flags_axis =
                ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels;
            ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_None, ImPlotAxisFlags_Lock);
            ImPlot::SetupAxis(ImAxis_X1, nullptr, flags_axis);
            ImPlot::SetupAxis(ImAxis_Y1, nullptr, flags_axis);
            ImPlot::SetupAxisLimits(ImAxis_X1,
                                    -GW::Constants::Range::Compass,
                                    GW::Constants::Range::Compass,
                                    ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1,
                                    -GW::Constants::Range::Compass,
                                    GW::Constants::Range::Compass,
                                    ImGuiCond_Always);

            PlotPoint(player_pos, player_pos, "player_pos", ImVec4{1.0F, 1.0F, 1.0F, 1.0F}, 5.0F);
            PlotPoint(player_pos, next_pos, "target", ImVec4{0.5F, 0.5F, 0.0F, 1.0F}, 5.0F);

            PlotRectangleLine(player_pos, rect.v1, rect.v2, "line1");
            PlotRectangleLine(player_pos, rect.v1, rect.v3, "line2");
            PlotRectangleLine(player_pos, rect.v4, rect.v2, "line3");
            PlotRectangleLine(player_pos, rect.v4, rect.v3, "line4");

            PlotCircle(player_pos, "circle", ImVec4{0.0, 0.0, 1.0, 1.0});

            PlotEnemies(player_pos, enemies, "enemiesAll", ImVec4{1.0F, 0.65F, 0.0, 1.0});

            const auto filtered_livings = GetEnemiesInGameRectangle(rect, enemies);
            PlotEnemies(player_pos, filtered_livings, "enemyInside", ImVec4{1.0, 0.0, 0.0, 1.0});
        }
        ImPlot::EndPlot();
    }
    ImGui::End();
}

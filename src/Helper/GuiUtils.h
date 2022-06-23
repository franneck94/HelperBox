#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include <Actions.h>
#include <Player.h>
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

void plot_shaded_rect(const float x1,
                      const float x2,
                      const float y1,
                      const float y2,
                      const float y3,
                      const float y4,
                      std::string_view label);

void plot_rectangle_line(const GW::GamePos &p1, const GW::GamePos &p2, std::string_view label);

void plot_point(const GW::GamePos &p, std::string_view label, const ImVec4 &color, const float width = 5.0F);

void plot_circle(const Player &player, std::string_view label, const ImVec4 &color);

void plot_enemies(const std::vector<GW::AgentLiving *> &living_agents, std::string_view label, const ImVec4 &color);

template <typename T, uint32_t N>
void DrawMap(const Player &player, const std::array<T, N> &moves, const uint32_t move_idx, std::string_view label)
{
    ImGui::SetNextWindowSize(ImVec2{400.0F, 400.0F}, ImGuiCond_FirstUseEver);
    const auto label_window = fmt::format("{}Window", label.data());
    if (ImGui::Begin(label_window.data(), nullptr, ImGuiWindowFlags_None))
    {
        const auto label_plot = fmt::format("{}Plot", label.data());
        if (ImPlot::BeginPlot(label_plot.data(), ImVec2{400.0F, 400.0F}, ImPlotFlags_None | ImPlotFlags_NoLegend))
        {
            auto next_pos = GW::GamePos{};

            if (move_idx < moves.size() - 1U && moves[move_idx].move_state == MoveState::WAIT)
                next_pos = moves[move_idx + 1U].pos;
            else
                next_pos = moves[move_idx].pos;

            const auto rect = GameRectangle(player.pos, next_pos, GW::Constants::Range::Spellcast);

            ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);

            const auto x_ = player.pos.x;
            const auto y_ = player.pos.y;
            plot_point(GW::GamePos{x_ + 4000.0F, y_, 0}, "border1", ImVec4{0.0F, 0.0F, 0.0F, 0.0F}, 1.0F);
            plot_point(GW::GamePos{x_, y_ + 4000.0F, 0}, "border2", ImVec4{0.0F, 0.0F, 0.0F, 0.0F}, 1.0F);
            plot_point(GW::GamePos{x_ - 4000.0F, y_, 0}, "border3", ImVec4{0.0F, 0.0F, 0.0F, 0.0F}, 1.0F);
            plot_point(GW::GamePos{x_, y_ - 4000.0F, 0}, "border4", ImVec4{0.0F, 0.0F, 0.0F, 0.0F}, 1.0F);

            plot_point(player.pos, "player", ImVec4{1.0F, 1.0F, 1.0F, 1.0F}, 5.0F);
            plot_point(next_pos, "target", ImVec4{0.5F, 0.5F, 0.0F, 1.0F}, 5.0F);

            plot_rectangle_line(rect.v1, rect.v2, "line1");
            plot_rectangle_line(rect.v1, rect.v3, "line2");
            plot_rectangle_line(rect.v4, rect.v2, "line3");
            plot_rectangle_line(rect.v4, rect.v3, "line4");

            plot_circle(player, "circle", ImVec4{0.0, 0.0, 1.0, 1.0});

            const auto living_agents = GetEnemiesInCompass();
            plot_enemies(living_agents, "enemiesAll", ImVec4{0.0, 1.0, 0.0, 1.0});

            const auto filtered_livings = GetEnemiesInGameRectangle(rect);
            plot_enemies(filtered_livings, "enemyInside", ImVec4{1.0, 0.0, 0.0, 1.0});
        }
        ImPlot::EndPlot();
    }
    ImGui::End();
}

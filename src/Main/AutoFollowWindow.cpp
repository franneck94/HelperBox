#include "stdafx.h"

#include <array>
#include <cstdint>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Player.h>

#include <fmt/format.h>

#include <HelperBox.h>

#include <Actions.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <Player.h>
#include <Types.h>
#include <Utils.h>

#include "AutoFollowWindow.h"

void AutoFollowAction::Update()
{
    if (action_state == ActionState::ACTIVE)
    {
        const auto routine_state = Routine();

        if (routine_state == RoutineState::FINISHED)
            action_state = ActionState::INACTIVE;
    }
}

RoutineState AutoFollowAction::Routine()
{
    if (!player || !player->target || !player->target->GetIsLivingType())
        return RoutineState::FINISHED;

    if (player->living->GetIsMoving())
        return RoutineState::ACTIVE;

    GW::Agents::GoPlayer(player->target);

    return RoutineState::ACTIVE;
}

void AutoFollowWindow::Draw(IDirect3DDevice9 *pDevice)
{
    UNREFERENCED_PARAMETER(pDevice);

    if (!visible)
        return;

    ImGui::SetNextWindowSize(ImVec2(100.0F, 100.0F), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("AutoFollowWindow", nullptr, GetWinFlags() | ImGuiWindowFlags_NoDecoration))
    {
        const auto width = ImGui::GetWindowWidth();
        auto_follow.Draw(ImVec2(width, 35.0F));
    }
    ImGui::End();
}

void AutoFollowWindow::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);

    if (IsLoading())
        return;

    if (!player.ValidateData())
        return;
    player.Update();

    auto_follow.Update();
}

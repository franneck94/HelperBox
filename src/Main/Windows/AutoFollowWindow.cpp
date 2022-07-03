#include "stdafx.h"

#include <array>
#include <cstdint>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Player.h>

#include <Base/HelperBox.h>

#include <Actions.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <MathUtils.h>
#include <PlayerData.h>
#include <Types.h>
#include <UwHelper.h>

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
    if (!player_data || !player_data->target || !player_data->target->GetIsLivingType())
        return RoutineState::FINISHED;

    if (player_data->living->GetIsMoving())
        return RoutineState::ACTIVE;

    GW::Agents::GoPlayer(player_data->target);

    return RoutineState::ACTIVE;
}

void AutoFollowWindow::Draw(IDirect3DDevice9 *)
{
    if (!visible)
        return;

    if (!player_data.ValidateData(HelperActivationConditions))
        return;

    ImGui::SetNextWindowSize(ImVec2(125.0F, 50.0F), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("AutoFollowWindow", nullptr, GetWinFlags() | ImGuiWindowFlags_NoDecoration))
    {
        const auto width = ImGui::GetWindowWidth();
        auto_follow.Draw(ImVec2(width, 35.0F));
    }
    ImGui::End();
}

void AutoFollowWindow::Update(float, const PlayerData &, const AgentLivingData &)
{
    if (IsLoading())
        return;

    if (!player_data.ValidateData(HelperActivationConditions))
        return;
    player_data.Update();

    auto_follow.Update();
}
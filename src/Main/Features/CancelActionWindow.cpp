#include <array>
#include <cstdint>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Player.h>

#include <Base/HelperBox.h>

#include <Actions.h>
#include <Helper.h>
#include <MathUtils.h>
#include <Types.h>

#include "CancelActionWindow.h"


void CancelActionWindow::Draw(IDirect3DDevice9 *)
{
    if (!visible)
        return;

    if (!HelperActivationConditions())
        return;

    ImGui::SetNextWindowSize(ImVec2(125.0F, 50.0F), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("CancelActionWindow", nullptr, GetWinFlags() | ImGuiWindowFlags_NoDecoration))
    {
        const auto width = ImGui::GetWindowWidth();
        if (ImGui::Button("Cancel Action", ImVec2(width, 35.0F)))
        {
            CancelMovement();
        }
    }
    ImGui::End();
}

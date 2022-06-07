#include "stdafx.h"

#include <array>
#include <cstdint>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Player.h>

#include <HelperBox.h>

#include <Actions.h>
#include <Helper.h>
#include <Types.h>
#include <Utils.h>

#include "CancelActionWindow.h"


void CancelActionWindow::Draw(IDirect3DDevice9 *pDevice)
{
    UNREFERENCED_PARAMETER(pDevice);

    if (!visible)
        return;

    if (IsLoading())
        return;

    ImGui::SetNextWindowSize(ImVec2(125.0F, 50.0F), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("CancelActionWindow", nullptr, GetWinFlags() | ImGuiWindowFlags_NoDecoration))
    {
        const auto width = ImGui::GetWindowWidth();
        if (ImGui::Button("Cancel Action", ImVec2(width, 35.0F)))
        {
            GW::CtoS::SendPacket(0x4, GAME_CMSG_CANCEL_MOVEMENT);
        }
    }
    ImGui::End();
}

void CancelActionWindow::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);
}

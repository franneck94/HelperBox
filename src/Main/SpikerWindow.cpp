#include "stdafx.h"

#include <array>
#include <cstdint>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/Context/GameContext.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Player.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/EffectMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/PlayerMgr.h>
#include <GWCA/Managers/SkillbarMgr.h>
#include <GWCA/Packets/Opcodes.h>

#include <fmt/format.h>

#include <HelperBox.h>
#include <Logger.h>
#include <Timer.h>

#include <Actions.h>
#include <Coloring.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <Player.h>
#include <Skillbars.h>
#include <Types.h>
#include <Utils.h>

#include "SpikerWindow.h"

void SpikerWindow::Draw(IDirect3DDevice9 *pDevice)
{
    UNREFERENCED_PARAMETER(pDevice);

    if (GW::Map::GetInstanceType() == GW::Constants::InstanceType::Loading)
        return;

    if (!visible)
        return;

    ImGui::SetNextWindowSize(WINDOW_SIZE, ImGuiCond_FirstUseEver);

    if (ImGui::Begin("SpikerWindow", nullptr, GetWinFlags()))
    {
        uint32_t idx = 0;
        for (const auto &foe : nearby_foes)
        {
            bool pushed = false;

            if (foe->GetIsHexed())
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0F, 10.0F, 1.0F, 255.0));
                pushed = true;
            }
            ImGui::Text("Agent: %d", foe->agent_id);
            ImGui::SameLine();
            const auto label = fmt::format("Target##{}", idx);
            if (ImGui::Button(label.data()))
            {
                Log::Info("Clicked");
                player.ChangeTarget(foe->agent_id);
            }

            if (pushed)
            {
                ImGui::PopStyleColor();
            }

            ++idx;
        }
    }

    ImGui::End();
}

void SpikerWindow::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);

    nearby_foes.clear();
    FilterAgentsByID(nearby_foes, GW::Constants::ModelID::UW::BladedAatxe);
}

#include "stdafx.h"

#include <GWCA/Constants/Constants.h>
#include <GWCA/GameContainers/Array.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Party.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Packets/StoC.h>

#include <Defines.h>
#include <HelperBox.h>

#include <Base/MainWindow.h>
#include <ChatCommands.h>
#include <EmoWindow.h>
#include <MesmerWindow.h>
#include <SettingsWindow.h>
#include <TerraWindow.h>

#include "HelperBoxSettings.h"

bool HelperBoxSettings::move_all = false;

void HelperBoxSettings::LoadModules(CSimpleIni *ini)
{
    SettingsWindow::Instance().sep = optional_modules.size();

    optional_modules.push_back(&ChatCommands::Instance());
    optional_modules.push_back(&EmoWindow::Instance());
    optional_modules.push_back(&MesmerWindow::Instance());
    optional_modules.push_back(&TerraWindow::Instance());
    optional_modules.push_back(&SettingsWindow::Instance());

    for (HelperBoxModule *module : optional_modules)
    {
        module->Initialize();
        module->LoadSettings(ini);
    }
}

void HelperBoxSettings::DrawSettingInternal()
{
    ImGui::Separator();
    const size_t cols = (size_t)floor(ImGui::GetWindowWidth() / (170.0f * ImGui::GetIO().FontGlobalScale));
    ImGui::Separator();
    ImGui::PushID("global_enable");
    ImGui::Text("Enable the following features:");
    ImGui::TextDisabled("Unticking will disable a feature. Requires HelperBox restart.");
    static std::vector<std::pair<const char *, bool *>> features{
        {"EMO", &use_emo},
        {"Mesmer", &use_spiker},
        {"Terra", &use_terra},
    };
    ImGui::Columns(static_cast<int>(cols), "global_enable_cols", false);
    size_t items_per_col = (size_t)ceil(features.size() / static_cast<float>(cols));
    size_t col_count = 0;
    for (auto feature : features)
    {
        ImGui::Checkbox(feature.first, feature.second);
        col_count++;
        if (col_count == items_per_col)
        {
            ImGui::NextColumn();
            col_count = 0;
        }
    }
    ImGui::Columns(1);
    ImGui::PopID();

    ImGui::Separator();
    if (ImGui::TreeNodeEx("Show the following in the main window:",
                          ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth))
    {

        std::vector<HelperBoxUIElement *> ui = HelperBox::Instance().GetUIElements();
        ImGui::Columns(static_cast<int>(cols), "menubuttons_cols", false);
        col_count = 0;
        std::vector<HelperBoxUIElement *> valid_elements;
        for (size_t i = 0; i < ui.size(); i++)
        {
            auto &window = ui[i];
            if (window->can_show_in_main_window)
            {
                valid_elements.push_back(window);
            }
        }
        std::sort(valid_elements.begin(),
                  valid_elements.end(),
                  [](const HelperBoxModule *lhs, const HelperBoxModule *rhs) {
                      return std::string(lhs->Name()).compare(rhs->Name()) < 0;
                  });
        items_per_col = (size_t)ceil(valid_elements.size() / static_cast<float>(cols));
        for (size_t i = 0; i < valid_elements.size(); i++)
        {
            auto window = valid_elements[i];
            col_count++;
            if (col_count == items_per_col)
            {
                ImGui::NextColumn();
                col_count = 0;
            }
        }
        ImGui::Columns(1);
        ImGui::TreePop();
    }
}

void HelperBoxSettings::LoadSettings(CSimpleIni *ini)
{
    HelperBoxModule::LoadSettings(ini);
    move_all = false;
    use_emo = ini->GetBoolValue(Name(), VAR_NAME(use_emo), use_emo);
    use_spiker = ini->GetBoolValue(Name(), VAR_NAME(use_spiker), use_spiker);
    use_terra = ini->GetBoolValue(Name(), VAR_NAME(use_terra), use_terra);
}

void HelperBoxSettings::SaveSettings(CSimpleIni *ini)
{
    HelperBoxModule::SaveSettings(ini);
    ini->SetBoolValue(Name(), VAR_NAME(use_emo), use_emo);
    ini->SetBoolValue(Name(), VAR_NAME(use_spiker), use_spiker);
    ini->SetBoolValue(Name(), VAR_NAME(use_terra), use_terra);
}

void HelperBoxSettings::Draw(IDirect3DDevice9 *)
{
    ImGui::GetStyle().WindowBorderSize = (move_all ? 1.0f : 0.0f);
}

void HelperBoxSettings::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);
}

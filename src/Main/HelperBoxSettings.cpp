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

#include <ChatCommands.h>
#include <ChestRunnerWindow.h>
#include <EmoWindow.h>
#include <MainWindow.h>
#include <SettingsWindow.h>

#include "HelperBoxSettings.h"

bool HelperBoxSettings::move_all = false;

void HelperBoxSettings::LoadModules(CSimpleIni *ini)
{
    SettingsWindow::Instance().sep_modules = optional_modules.size();

    optional_modules.push_back(&ChatCommands::Instance());
    // optional_modules.push_back(&ChestRunnerWindow::Instance());
    optional_modules.push_back(&EmoWindow::Instance());
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
    DrawFreezeSetting();
    const size_t cols = (size_t)floor(ImGui::GetWindowWidth() / (170.0f * ImGui::GetIO().FontGlobalScale));
    ImGui::Separator();
    ImGui::PushID("global_enable");
    ImGui::Text("Enable the following features:");
    ImGui::TextDisabled("Unticking will disable a feature. Requires HelperBox restart.");
    static std::vector<std::pair<const char *, bool *>> features{
        {"Chest", &use_chest},
        {"EMO", &use_emo},
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
            if ((window->IsWidget() || window->IsWindow()) && window->can_show_in_main_window)
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
            if (ImGui::Checkbox(window->Name(), &window->show_menubutton))
                MainWindow::Instance().pending_refresh_buttons = true;
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

void HelperBoxSettings::DrawFreezeSetting()
{
    ImGui::Checkbox("Unlock Move All", &move_all);
}

void HelperBoxSettings::LoadSettings(CSimpleIni *ini)
{
    HelperBoxModule::LoadSettings(ini);
    move_all = false;
    use_chest = ini->GetBoolValue(Name(), VAR_NAME(use_chest), use_chest);
    use_emo = ini->GetBoolValue(Name(), VAR_NAME(use_emo), use_emo);
}

void HelperBoxSettings::SaveSettings(CSimpleIni *ini)
{
    HelperBoxModule::SaveSettings(ini);
    ini->SetBoolValue(Name(), VAR_NAME(use_chest), use_chest);
    ini->SetBoolValue(Name(), VAR_NAME(use_emo), use_emo);
}

void HelperBoxSettings::Draw(IDirect3DDevice9 *)
{
    ImGui::GetStyle().WindowBorderSize = (move_all ? 1.0f : 0.0f);
}

void HelperBoxSettings::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);
}

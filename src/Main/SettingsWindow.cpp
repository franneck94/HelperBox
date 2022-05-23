#include "stdafx.h"

#include <GWCA/Managers/MapMgr.h>

#include <Defines.h>
#include <HelperBox.h>

#include <Base/HelperBoxSettings.h>
#include <ChatCommands.h>
#include <MainWindow.h>

#include "SettingsWindow.h"

void SettingsWindow::LoadSettings(CSimpleIni *ini)
{
    HelperBoxWindow::LoadSettings(ini);
    hide_when_entering_explorable =
        ini->GetBoolValue(Name(), VAR_NAME(hide_when_entering_explorable), hide_when_entering_explorable);
}

void SettingsWindow::SaveSettings(CSimpleIni *ini)
{
    HelperBoxWindow::SaveSettings(ini);
    ini->SetBoolValue(Name(), VAR_NAME(hide_when_entering_explorable), hide_when_entering_explorable);
}

void SettingsWindow::Draw(IDirect3DDevice9 *pDevice)
{
    UNREFERENCED_PARAMETER(pDevice);
    static GW::Constants::InstanceType last_instance_type = GW::Constants::InstanceType::Loading;
    GW::Constants::InstanceType instance_type = GW::Map::GetInstanceType();

    if (instance_type == GW::Constants::InstanceType::Loading)
        return;

    if (instance_type != last_instance_type)
    {
        if (hide_when_entering_explorable && instance_type == GW::Constants::InstanceType::Explorable)
            visible = false;
        last_instance_type = instance_type;
    }

    if (!visible)
        return;

    ImGui::SetNextWindowSize(ImVec2(768, 768), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(Name(), GetVisiblePtr(), GetWinFlags()))
    {
        drawn_settings.clear();
        ImColor sCol(102, 187, 238, 255);
        ImGui::Text("HelperBox");

        HelperBoxSettings::Instance().DrawFreezeSetting();
        ImGui::SameLine();
        ImGui::Checkbox("Hide Settings when entering explorable area", &hide_when_entering_explorable);

        ImGui::Text("General:");

        DrawSettingsSection(HelperBoxSettings::Instance().SettingsName());

        const std::vector<HelperBoxModule *> &optional_modules = HelperBoxSettings::Instance().GetOptionalModules();
        for (unsigned i = 0; i < optional_modules.size(); ++i)
        {
            if (i == sep)
                ImGui::Text("Components:");
            DrawSettingsSection(optional_modules[i]->SettingsName());
        }
    }

    float w = ImGui::GetWindowContentRegionWidth();

    if (ImGui::Button("Save Now", ImVec2(w, 0)))
    {
        HelperBox::Instance().SaveSettings();
    }

    if (ImGui::Button("Load Now", ImVec2(w, 0)))
    {
        HelperBox::Instance().OpenSettingsFile();
        HelperBox::Instance().LoadModuleSettings();
    }

    ImGui::End();
}

bool SettingsWindow::DrawSettingsSection(const char *section)
{
    const auto &callbacks = HelperBoxModule::GetSettingsCallbacks();
    const auto &icons = HelperBoxModule::GetSettingsIcons();

    const auto &settings_section = callbacks.find(section);
    if (settings_section == callbacks.end())
        return false;
    if (drawn_settings.find(section) != drawn_settings.end())
        return true; // Already drawn
    drawn_settings[section] = true;

    static char buf[128];
    sprintf(buf, "      %s", section);
    auto pos = ImGui::GetCursorScreenPos();
    const bool &is_showing = ImGui::CollapsingHeader(buf, ImGuiTreeNodeFlags_AllowItemOverlap);

    const char *icon = nullptr;
    auto it = icons.find(section);
    if (it != icons.end())
        icon = it->second;
    if (icon)
    {
        const auto &style = ImGui::GetStyle();
        const float text_offset_x = ImGui::GetTextLineHeightWithSpacing() + 4.0f; // TODO: find a proper number
        ImGui::GetWindowDrawList()->AddText(ImVec2(pos.x + text_offset_x, pos.y + style.ItemSpacing.y / 2),
                                            ImColor(style.Colors[ImGuiCol_Text]),
                                            icon);
    }

    if (is_showing)
        ImGui::PushID(section);
    size_t i = 0;
    for (auto &entry : settings_section->second)
    {
        ImGui::PushID(&settings_section->second);
        if (i && is_showing)
            ImGui::Separator();
        entry.second(&settings_section->first, is_showing);
        i++;
        ImGui::PopID();
    }
    if (is_showing)
        ImGui::PopID();
    return true;
}

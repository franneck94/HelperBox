#include "stdafx.h"

#include <HelperBox.h>
#include <Logger.h>

#include <MainWindow.h>

void MainWindow::LoadSettings(CSimpleIni *ini)
{
    HelperBoxWindow::LoadSettings(ini);
    pending_refresh_buttons = true;
    show_icons = false;
}

void MainWindow::SaveSettings(CSimpleIni *ini)
{
    HelperBoxWindow::SaveSettings(ini);
}

void MainWindow::DrawSettingInternal()
{
}

void MainWindow::RegisterSettingsContent()
{
    HelperBoxModule::RegisterSettingsContent(
        SettingsName(),
        Icon(),
        [this](const std::string *section, bool is_showing) {
            UNREFERENCED_PARAMETER(section);
            if (!is_showing)
                return;
            ImGui::Text("Main Window Visibility");
            ShowVisibleRadio();
            DrawSizeAndPositionSettings();
            DrawSettingInternal();
        },
        1.0F);
}

void MainWindow::RefreshButtons()
{
    pending_refresh_buttons = false;
    const std::vector<HelperBoxUIElement *> &ui = HelperBox::Instance().GetUIElements();
    modules_to_draw.clear();
    for (auto &ui_module : ui)
    {
        if (!ui_module->show_menubutton)
        {
            continue;
        }

        float weighting = GetModuleWeighting(ui_module);
        auto it = modules_to_draw.begin();
        for (it = modules_to_draw.begin(); it != modules_to_draw.end(); it++)
        {
            if (it->first > weighting)
            {
                break;
            }
        }
        modules_to_draw.insert(it, {weighting, ui_module});
    }
}

void MainWindow::Draw(IDirect3DDevice9 *device)
{
    if (!visible)
    {
        return;
    }

    if (pending_refresh_buttons)
    {
        RefreshButtons();
    }

    static bool open = true;
    ImGui::SetNextWindowSize(ImVec2(110.0f, 300.0f), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(Name(), show_closebutton ? &open : nullptr, GetWinFlags()))
    {
        bool drawn = false;
        const size_t msize = modules_to_draw.size();
        for (size_t i = 0; i < msize; i++)
        {
            ImGui::PushID(static_cast<int>(i));
            if (drawn)
            {
                ImGui::Separator();
            }

            drawn = true;
            auto &ui_module = modules_to_draw[i].second;
            if (ui_module->DrawTabButton(device, show_icons, true, center_align_text))
            {
                if (one_panel_at_time_only && ui_module->visible && ui_module->IsWindow())
                {
                    for (auto &ui_module2 : modules_to_draw)
                    {
                        if (ui_module2.second == ui_module)
                        {
                            continue;
                        }
                        if (!ui_module2.second->IsWindow())
                        {
                            continue;
                        }
                        ui_module2.second->visible = false;
                    }
                }
            }
            ImGui::PopID();
        }
    }
    ImGui::End();

    if (!open)
    {
        HelperBox::Instance().StartSelfDestruct();
    }
}

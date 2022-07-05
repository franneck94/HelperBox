#include <filesystem>

#include <Logger.h>
#include <Path.h>

#include <SimpleIni.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "HelperBoxTheme.h"

namespace
{
static constexpr auto WindowPositionsFilename = L"Layout.ini";
};

HelperBoxTheme::HelperBoxTheme()
{
    ini_style = DefaultTheme();
}

ImGuiStyle HelperBoxTheme::DefaultTheme()
{
    ImGuiStyle style = ImGuiStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 2.0f;
    style.ScrollbarSize = 16.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabMinSize = 17.0f;
    style.GrabRounding = 2.0f;

    return style;
}

void HelperBoxTheme::Terminate()
{
    HelperBoxModule::Terminate();
    if (inifile)
    {
        inifile->Reset();
        delete inifile;
        inifile = nullptr;
    }
}

void HelperBoxTheme::LoadSettings(CSimpleIni *ini)
{
    HelperBoxModule::LoadSettings(ini);

    if (!inifile)
        inifile = new CSimpleIni(false, false, false);

    layout_dirty = true;
}

void HelperBoxTheme::SaveUILayout()
{
    CSimpleIni *ini = GetLayoutIni();
    const char *window_ini_section = "Windows";
    ImVector<ImGuiWindow *> &windows = ImGui::GetCurrentContext()->Windows;

    for (ImGuiWindow *window : windows)
    {
        char key[128];
        snprintf(key, 128, "_%s_X", window->Name);
        ini->SetDoubleValue(window_ini_section, key, window->Pos.x);
        snprintf(key, 128, "_%s_Y", window->Name);
        ini->SetDoubleValue(window_ini_section, key, window->Pos.y);
        snprintf(key, 128, "_%s_W", window->Name);
        ini->SetDoubleValue(window_ini_section, key, window->SizeFull.x);
        snprintf(key, 128, "_%s_H", window->Name);
        ini->SetDoubleValue(window_ini_section, key, window->SizeFull.y);
        snprintf(key, 128, "_%s_Collapsed", window->Name);
        ini->SetBoolValue(window_ini_section, key, window->Collapsed);
    }

    ini->SaveFile(GetPath(WindowPositionsFilename).c_str());
}

CSimpleIni *HelperBoxTheme::GetLayoutIni()
{
    if (!windows_ini)
        windows_ini = new CSimpleIni(false, false, false);

    windows_ini->Reset();
    std::wstring filename = GetPath(WindowPositionsFilename);

    if (std::filesystem::exists(filename.c_str()))
        windows_ini->LoadFile(filename.c_str());
    else
        Log::LogW(L"File %s doesn't exist.", filename.c_str());

    return windows_ini;
}

void HelperBoxTheme::LoadUILayout()
{
    ImGui::GetStyle() = ini_style;
    ImGui::GetIO().FontGlobalScale = font_global_scale;
    CSimpleIni *ini = GetLayoutIni();
    ImVector<ImGuiWindow *> &windows = ImGui::GetCurrentContext()->Windows;
    const char *window_ini_section = "Windows";

    for (ImGuiWindow *window : windows)
    {
        if (!window)
            continue;
        ImVec2 pos = window->Pos;
        ImVec2 size = window->Size;
        char key[128];
        snprintf(key, 128, "_%s_X", window->Name);
        pos.x = static_cast<float>(ini->GetDoubleValue(window_ini_section, key, pos.x));
        snprintf(key, 128, "_%s_Y", window->Name);
        pos.y = static_cast<float>(ini->GetDoubleValue(window_ini_section, key, pos.y));
        snprintf(key, 128, "_%s_W", window->Name);
        size.x = static_cast<float>(ini->GetDoubleValue(window_ini_section, key, size.x));
        snprintf(key, 128, "_%s_H", window->Name);
        size.y = static_cast<float>(ini->GetDoubleValue(window_ini_section, key, size.y));
        snprintf(key, 128, "_%s_Collapsed", window->Name);

        bool collapsed = ini->GetBoolValue(window_ini_section, key, false);
        ImGui::SetWindowPos(window, pos);
        ImGui::SetWindowSize(window, size);
        ImGui::SetWindowCollapsed(window, collapsed);
    }

    layout_dirty = false;
}

void HelperBoxTheme::SaveSettings(CSimpleIni *ini)
{
    HelperBoxModule::SaveSettings(ini);
    SaveUILayout();
}

void HelperBoxTheme::Draw(IDirect3DDevice9 *)
{
    if (layout_dirty)
        LoadUILayout();
}

void HelperBoxTheme::DrawSettingInternal()
{
}

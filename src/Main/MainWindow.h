#pragma once

#include <Base/HelperBoxWindow.h>

class MainWindow : public HelperBoxWindow
{
    MainWindow()
    {
        visible = true;
        can_show_in_main_window = false;
    };
    ~MainWindow(){};

public:
    static MainWindow &Instance()
    {
        static MainWindow instance;
        return instance;
    }

    const char *Name() const override
    {
        return "HelperBox";
    }

    const char *SettingsName() const override
    {
        return "HelperBox Settings";
    }

    void Draw(IDirect3DDevice9 *pDevice) override;
    void LoadSettings(CSimpleIni *ini) override;
    void SaveSettings(CSimpleIni *ini) override;
    void DrawSettingInternal() override;
    void RegisterSettingsContent() override;

    void RefreshButtons();
    bool pending_refresh_buttons = true;

private:
    bool one_panel_at_time_only = false;
    bool show_icons = true;
    bool center_align_text = false;

    float GetModuleWeighting(HelperBoxUIElement *m)
    {
        auto found = module_weightings.find(m->Name());
        return found == module_weightings.end() ? 1.0f : found->second;
    }
    std::vector<std::pair<float, HelperBoxUIElement *>> modules_to_draw{};
    const std::unordered_map<std::string, float> module_weightings{{"Chests", 0.5f}, {"Emo", 0.52f}};
};

#pragma once

#include <HelperBoxUIElement.h>

class HelperBoxTheme : public HelperBoxUIElement
{
    HelperBoxTheme();
    ~HelperBoxTheme()
    {
        if (windows_ini)
        {
            delete windows_ini;
        }
        if (inifile)
        {
            delete inifile;
        }
    }

public:
    static HelperBoxTheme &Instance()
    {
        static HelperBoxTheme instance;
        return instance;
    }

    const char *Name() const override
    {
        return "Theme";
    }
    const char *Icon() const override
    {
        return nullptr;
    }

    void Terminate() override;
    void LoadSettings(CSimpleIni *ini) override;
    void SaveSettings(CSimpleIni *ini) override;
    void Draw(IDirect3DDevice9 *device) override;
    void ShowVisibleRadio() override{};

    void SaveUILayout();
    void LoadUILayout();

    void DrawSettingInternal() override;

    CSimpleIni *GetLayoutIni();

private:
    ImGuiStyle DefaultTheme();

    float font_global_scale = 1.0;
    ImGuiStyle ini_style;
    bool layout_dirty = false;

    CSimpleIni *inifile = nullptr;
    CSimpleIni *windows_ini = nullptr;
};

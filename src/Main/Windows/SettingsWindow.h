#pragma once

#include <Base/HelperBoxWindow.h>

class SettingsWindow : public HelperBoxWindow
{
    SettingsWindow(){};
    ~SettingsWindow(){};

public:
    static SettingsWindow &Instance()
    {
        static SettingsWindow instance;
        return instance;
    }

    const char *Name() const override
    {
        return "Settings";
    }

    void Draw(IDirect3DDevice9 *pDevice) override;
    bool DrawSettingsSection(const char *section);

    size_t sep = 0;

private:
    std::map<std::string, bool> drawn_settings{};
};

#pragma once

#include <Base/HelperBoxModule.h>

class HelperBoxUIElement : public HelperBoxModule
{
    friend class HelperBoxSettings;

public:
    virtual void Draw(IDirect3DDevice9 *){};
    virtual const char *UIName() const;

    virtual void Initialize() override;
    virtual void Terminate() override;

    virtual void LoadSettings(CSimpleIni *ini) override;
    virtual void SaveSettings(CSimpleIni *ini) override;
    virtual bool DrawTabButton(IDirect3DDevice9 *device,
                               bool show_icon = true,
                               bool show_text = true,
                               bool center_align_text = true);

    virtual bool ToggleVisible()
    {
        return visible = !visible;
    }

    virtual bool IsWindow() const
    {
        return false;
    }
    virtual bool IsWidget() const
    {
        return false;
    }
    virtual bool ShowOnWorldMap() const
    {
        return false;
    }

    virtual const char *TypeName() const
    {
        return "ui element";
    }

    virtual void RegisterSettingsContent() override;

    void DrawSizeAndPositionSettings();

    bool visible = false;
    bool lock_move = false;
    bool lock_size = false;
    bool show_menubutton = false;
    bool show_closebutton = true;

    bool *GetVisiblePtr(bool force_show = false)
    {
        if (!has_closebutton || show_closebutton || force_show)
            return &visible;
        return nullptr;
    }

protected:
    bool can_show_in_main_window = true;
    bool has_closebutton = false;
    bool is_resizable = true;
    bool is_movable = true;

    virtual void ShowVisibleRadio();
    IDirect3DTexture9 *button_texture = nullptr;
};

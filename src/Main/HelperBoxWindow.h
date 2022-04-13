#pragma once

#include <Defines.h>
#include <HelperBoxUIElement.h>

class HelperBoxWindow : public HelperBoxUIElement
{
public:
    bool IsWindow() const override
    {
        return true;
    }
    const char *TypeName() const override
    {
        return "window";
    }

    virtual void Initialize() override
    {
        HelperBoxUIElement::Initialize();
        has_closebutton = true;
    }

    virtual void LoadSettings(CSimpleIni *ini) override
    {
        HelperBoxUIElement::LoadSettings(ini);
        lock_move = ini->GetBoolValue(Name(), VAR_NAME(lock_move), lock_move);
        lock_size = ini->GetBoolValue(Name(), VAR_NAME(lock_size), lock_size);
        show_closebutton = ini->GetBoolValue(Name(), VAR_NAME(show_closebutton), show_closebutton);
    }

    virtual void SaveSettings(CSimpleIni *ini) override
    {
        HelperBoxUIElement::SaveSettings(ini);
        ini->SetBoolValue(Name(), VAR_NAME(lock_move), lock_move);
        ini->SetBoolValue(Name(), VAR_NAME(lock_size), lock_size);
        ini->SetBoolValue(Name(), VAR_NAME(show_closebutton), show_closebutton);
    }

    ImGuiWindowFlags GetWinFlags(ImGuiWindowFlags flags = ImGuiWindowFlags_None) const;
};

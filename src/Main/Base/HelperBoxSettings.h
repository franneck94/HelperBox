#pragma once

#include <GWCA/Constants/Maps.h>

#include <Base/HelperBoxUIElement.h>

class HelperBoxSettings : public HelperBoxUIElement
{
    HelperBoxSettings(){};
    HelperBoxSettings(const HelperBoxSettings &) = delete;
    ~HelperBoxSettings(){};

public:
    static HelperBoxSettings &Instance()
    {
        static HelperBoxSettings instance;
        return instance;
    }

    const char *Name() const override
    {
        return "HelperBox Settings";
    }

    const char *Icon() const override
    {
        return nullptr;
    }

    void LoadModules(CSimpleIni *ini);

    void Update(float delta) override;

    void LoadSettings(CSimpleIni *ini) override;
    void SaveSettings(CSimpleIni *ini) override;
    void DrawSettingInternal() override;
    void Draw(IDirect3DDevice9 *) override;
    void ShowVisibleRadio() override{};

    void DrawFreezeSetting();

    const std::vector<HelperBoxModule *> &GetOptionalModules() const
    {
        return optional_modules;
    }

    static bool move_all;

private:
    std::vector<HelperBoxModule *> optional_modules;

    bool use_emo = true;
    bool use_spiker = true;
    bool use_terra = true;
};
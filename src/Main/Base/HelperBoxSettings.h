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
        return "HelperBox";
    }

    void LoadModules(CSimpleIni *ini);

    void Update(float delta) override;

    void LoadSettings(CSimpleIni *ini) override;
    void SaveSettings(CSimpleIni *ini) override;
    void DrawSettingInternal() override;
    void Draw(IDirect3DDevice9 *) override;
    void ShowVisibleRadio() override{};

    const std::vector<HelperBoxModule *> &GetOptionalModules() const
    {
        return optional_modules;
    }

    static bool move_all;

private:
    std::vector<HelperBoxModule *> optional_modules;

    bool use_emo = true;
    bool use_mainteam = true;
    bool use_terra = true;
    bool use_follow = true;
    bool use_cancel = true;
};

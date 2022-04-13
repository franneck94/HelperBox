#pragma once

#include "stdafx.h"

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <string_view>

#include <Player.h>

#include <HelperBoxWindow.h>

class EmoWindow : public HelperBoxWindow
{
public:
    EmoWindow() : player(){};
    ~EmoWindow(){};

    static EmoWindow &Instance()
    {
        static EmoWindow instance;
        return instance;
    }

    const char *Name() const override
    {
        return "EmoWindow";
    }

    void Initialize() override;
    void Draw(IDirect3DDevice9 *pDevice) override;
    void Update(float delta) override;
    void LoadSettings(CSimpleIni *ini) override;
    void SaveSettings(CSimpleIni *ini) override;

private:
    void EmoSkillRoutine();
    bool EmoBondTankRoutine();
    bool EmoBondPlayerRoutine();
    bool EmoFusePull();

    Player player;
};

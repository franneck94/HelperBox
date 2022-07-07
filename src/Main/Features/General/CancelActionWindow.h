#pragma once

#include <cstdint>

#include <GWCA/GameEntities/Agent.h>

#include <Base/HelperBoxWindow.h>

#include <Actions.h>
#include <PlayerData.h>
#include <Types.h>

#include <SimpleIni.h>
#include <imgui.h>

class CancelActionWindow : public HelperBoxWindow
{
public:
    CancelActionWindow(){};
    ~CancelActionWindow(){};

    static CancelActionWindow &Instance()
    {
        static CancelActionWindow instance;
        return instance;
    }

    const char *Name() const override
    {
        return "CancelActionWindow";
    }

    void Draw(IDirect3DDevice9 *pDevice) override;
    void Update(float delta, const AgentLivingData &) override;
};

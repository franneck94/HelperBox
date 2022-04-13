#pragma once

#include <stdafx.h>

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

#include <HelperBoxWindow.h>

#include <Actions.h>
#include <Helper.h>
#include <Skillbars.h>

class ChestRunnerWindow : public HelperBoxWindow
{
public:
    ChestRunnerWindow() : actions(std::vector<std::unique_ptr<Action>>{}){};
    ~ChestRunnerWindow(){};

    static ChestRunnerWindow &Instance()
    {
        static ChestRunnerWindow instance;
        return instance;
    }

    const char *Name() const override
    {
        return "Helper";
    }

    void Initialize() override;
    void Draw(IDirect3DDevice9 *pDevice) override;
    void Update(float delta) override;
    void LoadSettings(CSimpleIni *ini) override;
    void SaveSettings(CSimpleIni *ini) override;

private:
    void AddChestActions();
    bool ChestMain();
    bool HelperMain();

    RangerRunnerSkillbar skillbar{};

    boolean start_helper = false;
    std::vector<std::unique_ptr<Action>> actions;
    uint32_t action_idx = 0;
};

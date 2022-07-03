#include "stdafx.h"

#include <GWCA/Constants/Constants.h>
#include <GWCA/GameContainers/Array.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Party.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Packets/StoC.h>

#include <Defines.h>

#include <Base/ChatCommands.h>
#include <Base/HelperBox.h>
#include <Base/MainWindow.h>

#include <Windows/AutoFollowWindow.h>
#include <Windows/CancelActionWindow.h>
#include <Windows/DbWindow.h>
#include <Windows/DhuumStatsWindow.h>
#include <Windows/EmoWindow.h>
#include <Windows/MainteamWindow.h>
#include <Windows/SettingsWindow.h>
#include <Windows/TerraWindow.h>

#include "HelperBoxSettings.h"

bool HelperBoxSettings::move_all = false;

void HelperBoxSettings::LoadModules(CSimpleIni *ini)
{
    SettingsWindow::Instance().sep = optional_modules.size();

    optional_modules.push_back(&ChatCommands::Instance());
    optional_modules.push_back(&EmoWindow::Instance());
    optional_modules.push_back(&DbWindow::Instance());
    optional_modules.push_back(&MainteamWindow::Instance());
    optional_modules.push_back(&TerraWindow::Instance());
    // optional_modules.push_back(&AutoFollowWindow::Instance());
    optional_modules.push_back(&DhuumStatsWindow::Instance());
    // optional_modules.push_back(&CancelActionWindow::Instance());
    optional_modules.push_back(&SettingsWindow::Instance());

    for (HelperBoxModule *module : optional_modules)
    {
        module->Initialize();
        module->LoadSettings(ini);
    }
}

void HelperBoxSettings::DrawSettingInternal()
{
}

void HelperBoxSettings::LoadSettings(CSimpleIni *ini)
{
    HelperBoxModule::LoadSettings(ini);
    move_all = false;
    use_emo = ini->GetBoolValue(Name(), VAR_NAME(use_emo), use_emo);
    use_mainteam = ini->GetBoolValue(Name(), VAR_NAME(use_mainteam), use_mainteam);
    use_terra = ini->GetBoolValue(Name(), VAR_NAME(use_terra), use_terra);
}

void HelperBoxSettings::SaveSettings(CSimpleIni *ini)
{
    HelperBoxModule::SaveSettings(ini);
    ini->SetBoolValue(Name(), VAR_NAME(use_emo), use_emo);
    ini->SetBoolValue(Name(), VAR_NAME(use_mainteam), use_mainteam);
    ini->SetBoolValue(Name(), VAR_NAME(use_terra), use_terra);
}

void HelperBoxSettings::Draw(IDirect3DDevice9 *)
{
    ImGui::GetStyle().WindowBorderSize = (move_all ? 1.0f : 0.0f);
}

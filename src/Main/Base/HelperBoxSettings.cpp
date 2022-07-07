#include <Base/HelperBox.h>
#include <Base/MainWindow.h>
#include <Base/SettingsWindow.h>
#include <Defines.h>
#include <Features/General/AutoFollowWindow.h>
#include <Features/General/CancelActionWindow.h>
#include <Features/General/ChatCommands.h>
#include <Features/Uw/DbWindow.h>
#include <Features/Uw/DhuumStatsWindow.h>
#include <Features/Uw/EmoWindow.h>
#include <Features/Uw/MainteamWindow.h>
#include <Features/Uw/TerraWindow.h>

#include <SimpleIni.h>

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

void HelperBoxSettings::LoadSettings(CSimpleIni *ini)
{
    HelperBoxModule::LoadSettings(ini);
    move_all = false;
    use_emo = ini->GetBoolValue(Name(), VAR_NAME(use_emo), use_emo);
    use_mainteam = ini->GetBoolValue(Name(), VAR_NAME(use_mainteam), use_mainteam);
    use_terra = ini->GetBoolValue(Name(), VAR_NAME(use_terra), use_terra);
    use_db = ini->GetBoolValue(Name(), VAR_NAME(use_db), use_db);
    use_dhuum_stats = ini->GetBoolValue(Name(), VAR_NAME(use_dhuum_stats), use_dhuum_stats);
    use_follow = ini->GetBoolValue(Name(), VAR_NAME(use_follow), use_follow);
    use_cancel = ini->GetBoolValue(Name(), VAR_NAME(use_cancel), use_cancel);
}

void HelperBoxSettings::SaveSettings(CSimpleIni *ini)
{
    HelperBoxModule::SaveSettings(ini);
    ini->SetBoolValue(Name(), VAR_NAME(use_emo), use_emo);
    ini->SetBoolValue(Name(), VAR_NAME(use_mainteam), use_mainteam);
    ini->SetBoolValue(Name(), VAR_NAME(use_terra), use_terra);
    ini->SetBoolValue(Name(), VAR_NAME(use_db), use_db);
    ini->SetBoolValue(Name(), VAR_NAME(use_dhuum_stats), use_dhuum_stats);
    ini->SetBoolValue(Name(), VAR_NAME(use_follow), use_follow);
    ini->SetBoolValue(Name(), VAR_NAME(use_cancel), use_cancel);
}

void HelperBoxSettings::Draw(IDirect3DDevice9 *)
{
    ImGui::GetStyle().WindowBorderSize = (move_all ? 1.0f : 0.0f);
}

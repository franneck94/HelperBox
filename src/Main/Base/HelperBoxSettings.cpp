#include <Base/HelperBox.h>
#include <Base/MainWindow.h>
#include <Base/SettingsWindow.h>
#include <Defines.h>
#include <Features/General/AutoFollowWindow.h>
#include <Features/General/ChatCommands.h>
#include <Features/Uw/UwDhuumBitch.h>
#include <Features/Uw/UwDhuumStats.h>
#include <Features/Uw/UwEmo.h>
#include <Features/Uw/UwMesmer.h>
#include <Features/Uw/UwRanger.h>

#include <SimpleIni.h>

#include "HelperBoxSettings.h"

bool HelperBoxSettings::move_all = false;

void HelperBoxSettings::LoadModules(CSimpleIni *ini)
{
    SettingsWindow::Instance().sep = optional_modules.size();

    //General
    optional_modules.push_back(&ChatCommands::Instance());
    optional_modules.push_back(&SettingsWindow::Instance());
    // optional_modules.push_back(&AutoFollowWindow::Instance());

    // UW
    optional_modules.push_back(&UwEmo::Instance());
    optional_modules.push_back(&UwDhuumBitch::Instance());
    optional_modules.push_back(&UwMesmer::Instance());
    optional_modules.push_back(&UwRanger::Instance());
    optional_modules.push_back(&UwDhuumStats::Instance());

    for (auto module : optional_modules)
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
    ImGui::GetStyle().WindowBorderSize = (move_all ? 1.0F : 0.0F);
}

#include "stdafx.h"

#include <Base/HelperBoxModule.h>
#include <HelperBox.h>

namespace
{
static std::unordered_map<std::string, SectionDrawCallbackList> settings_draw_callbacks{};
static std::unordered_map<std::string, const char *> settings_icons{};
static std::unordered_map<std::string, HelperBoxModule *> modules_loaded{};
} // namespace

const std::unordered_map<std::string, SectionDrawCallbackList> &HelperBoxModule::GetSettingsCallbacks()
{
    return settings_draw_callbacks;
}

const std::unordered_map<std::string, const char *> &HelperBoxModule::GetSettingsIcons()
{
    return settings_icons;
}

void HelperBoxModule::Initialize()
{
    if (!HelperBox::Instance().RegisterModule(this))
        return;
    RegisterSettingsContent();
}

void HelperBoxModule::RegisterSettingsContent()
{
    if (!HasSettings())
        return;
    RegisterSettingsContent(
        SettingsName(),
        Icon(),
        [this](const std::string *section, bool is_showing) {
            UNREFERENCED_PARAMETER(section);
            if (is_showing)
                DrawSettingInternal();
        },
        1.0);
}

void HelperBoxModule::RegisterSettingsContent(const char *section,
                                              const char *,
                                              SectionDrawCallback callback,
                                              float weighting)
{
    if (settings_draw_callbacks.find(section) == settings_draw_callbacks.end())
    {
        settings_draw_callbacks.emplace(section, SectionDrawCallbackList());
    }
    auto it = settings_draw_callbacks[section].begin();
    for (it = settings_draw_callbacks[section].begin(); it != settings_draw_callbacks[section].end(); it++)
    {
        if (it->first > weighting)
            break;
    }
    settings_draw_callbacks[section].insert(it, {weighting, callback});
}

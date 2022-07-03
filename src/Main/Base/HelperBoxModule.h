#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include <AgentLivingData.h>
#include <PlayerData.h>

typedef std::function<void(const std::string *section, bool is_showing)> SectionDrawCallback;
typedef std::vector<std::pair<float, SectionDrawCallback>> SectionDrawCallbackList;

class HelperBoxModule
{
public:
    HelperBoxModule()
    {
    }
    virtual ~HelperBoxModule(){};

public:
    virtual const char *Name() const = 0;

    virtual const char *SettingsName() const
    {
        return Name();
    }

    virtual void RegisterSettingsContent();

    static const std::unordered_map<std::string, SectionDrawCallbackList> &GetSettingsCallbacks();
    static const std::unordered_map<std::string, const char *> &GetSettingsIcons();

    static const std::unordered_map<std::string, HelperBoxModule *> *GetModulesLoaded();
    virtual void Initialize();

    virtual void Terminate(){};
    virtual void Update(float, const AgentLivingData &){};

    virtual void LoadSettings(CSimpleIni *){};
    virtual void SaveSettings(CSimpleIni *){};
    virtual void DrawSettingInternal(){};

    static void RegisterSettingsContent(const char *section, SectionDrawCallback callback);
};

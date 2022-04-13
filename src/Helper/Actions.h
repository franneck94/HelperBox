#pragma once

#include <cstdint>
#include <string_view>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/GameContainers/GamePos.h>

#include <Helper.h>
#include <Skillbars.h>
#include <Types.h>
#include <Utils.h>

ActionState SafeResign();

ActionState SafeTravel(const GW::Constants::MapID target_map,
                       const GW::Constants::MapRegion target_region,
                       const GW::Constants::MapLanguage target_language);

ActionState SafeWalk(GW::GamePos target_position);

ActionState SafeUseSkill(const uint32_t skill_idx, const uint32_t target = 0, const uint32_t call_target = 0);

ActionState SafeLoadSkillTemplate(std::string_view code);

ActionState SafeChangeTarget(const TargetType type);

ActionState SafeGotoTarget();

ActionState SafeOpenChest();

class Action
{
public:
    const std::string_view name;

    Action(std::string_view name_) : name(name_){};
    virtual ~Action(){};

    virtual ActionState action_function() = 0;

    const ActionState operator()()
    {
        return action_function();
    }
};

class TravelAction : public Action
{
public:
    TravelAction(std::string_view name_,
                 const GW::Constants::MapID target_map_,
                 const GW::Constants::MapRegion target_region_,
                 const GW::Constants::MapLanguage target_language_)
        : Action(name_), target_map(target_map_), target_region(target_region_), target_language(target_language_){};
    ~TravelAction(){};

    ActionState action_function() override
    {
        return SafeTravel(target_map, target_region, target_language);
    }


private:
    GW::Constants::MapID target_map;
    GW::Constants::MapRegion target_region;
    GW::Constants::MapLanguage target_language;
};

class WalkAction : public Action
{
public:
    WalkAction(std::string_view name_, const GW::GamePos &target_position_)
        : Action(name_), target_position(target_position_){};
    ~WalkAction(){};

    ActionState action_function() override
    {
        return SafeWalk(target_position);
    }

private:
    GW::GamePos target_position;
};

class UseSkillAction : public Action
{
public:
    UseSkillAction(std::string_view name_,
                   const uint32_t skill_idx_,
                   const uint32_t target_ = 0,
                   const uint32_t call_target_ = 0)
        : Action(name_), skill_idx(skill_idx_), target(target_), call_target(call_target_){};
    ~UseSkillAction(){};

    ActionState action_function() override
    {
        return SafeUseSkill(skill_idx, target, call_target);
    }

private:
    uint32_t skill_idx;
    uint32_t target;
    uint32_t call_target;
};

class LoadSkillTemplateAction : public Action
{
public:
    LoadSkillTemplateAction(std::string_view name_, std::string_view code_) : Action(name_), code(code_){};
    ~LoadSkillTemplateAction(){};

    ActionState action_function()
    {
        return SafeLoadSkillTemplate(code);
    }

private:
    std::string_view code;
};

class ChangeTargetAction : public Action
{
public:
    ChangeTargetAction(std::string_view name_, TargetType type_) : Action(name_), type(type_){};
    ~ChangeTargetAction(){};

    ActionState action_function() override
    {
        return SafeChangeTarget(type);
    }

private:
    TargetType type;
};

class GotoTargetAction : public Action
{
public:
    GotoTargetAction(std::string_view name_) : Action(name_){};
    ~GotoTargetAction(){};

    ActionState action_function() override
    {
        return SafeGotoTarget();
    }
};

class OpenChestAction : public Action
{
public:
    OpenChestAction(std::string_view name_) : Action(name_){};
    ~OpenChestAction(){};

    ActionState action_function() override
    {
        return SafeOpenChest();
    }
};

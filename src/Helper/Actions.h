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

enum class ActionType
{
    NONE,
    TRAVEL,
    WALK,
    USE_SKILL,
    LOAD_SKILLS,
    CHANGE_TARGET,
    GOTO_TARGET,
    OPEN_CHEST,
    RESIGN
};

RoutineState SafeTravel(const GW::Constants::MapID target_map,
                        const GW::Constants::MapRegion target_region = GW::Constants::MapRegion::European,
                        const GW::Constants::MapLanguage target_language = GW::Constants::MapLanguage::Polish);

RoutineState SafeWalk(GW::GamePos target_position, const bool reset = false);

RoutineState SafeUseSkill(const uint32_t skill_idx, const uint32_t target = 0, const uint32_t call_target = 0);

RoutineState SafeLoadSkillTemplate(std::string_view code);

RoutineState SafeChangeTarget(const TargetType type);

RoutineState SafeGotoTarget();

RoutineState SafeOpenChest();

RoutineState SafeResign(bool issue_resign);

class Action
{
public:
    const std::string_view name;
    const ActionType type;

    Action(std::string_view name_, ActionType type_) : name(name_), type(type_){};
    virtual ~Action(){};

    virtual RoutineState action_function(const bool reset) = 0;

    const RoutineState operator()(const bool reset)
    {
        return action_function(reset);
    }
};

class TravelAction : public Action
{
public:
    TravelAction(std::string_view name_,
                 const GW::Constants::MapID target_map_,
                 const GW::Constants::MapRegion target_region_,
                 const GW::Constants::MapLanguage target_language_)
        : Action(name_, ActionType::TRAVEL), target_map(target_map_), target_region(target_region_),
          target_language(target_language_){};
    ~TravelAction(){};

    RoutineState action_function(const bool) override
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
        : Action(name_, ActionType::WALK), target_position(target_position_){};
    ~WalkAction(){};

    RoutineState action_function(const bool reset) override
    {
        return SafeWalk(target_position, reset);
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
        : Action(name_, ActionType::USE_SKILL), skill_idx(skill_idx_), target(target_), call_target(call_target_){};
    ~UseSkillAction(){};

    RoutineState action_function(const bool) override
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
    LoadSkillTemplateAction(std::string_view name_, std::string_view code_)
        : Action(name_, ActionType::LOAD_SKILLS), code(code_){};
    ~LoadSkillTemplateAction(){};

    RoutineState action_function(const bool)
    {
        return SafeLoadSkillTemplate(code);
    }

private:
    std::string_view code;
};

class ChangeTargetAction : public Action
{
public:
    ChangeTargetAction(std::string_view name_, TargetType type_)
        : Action(name_, ActionType::CHANGE_TARGET), type(type_){};
    ~ChangeTargetAction(){};

    RoutineState action_function(const bool) override
    {
        return SafeChangeTarget(type);
    }

private:
    TargetType type;
};

class GotoTargetAction : public Action
{
public:
    GotoTargetAction(std::string_view name_) : Action(name_, ActionType::GOTO_TARGET){};
    ~GotoTargetAction(){};

    RoutineState action_function(const bool) override
    {
        return SafeGotoTarget();
    }
};

class OpenChestAction : public Action
{
public:
    OpenChestAction(std::string_view name_) : Action(name_, ActionType::OPEN_CHEST){};
    ~OpenChestAction(){};

    RoutineState action_function(const bool) override
    {
        return SafeOpenChest();
    }
};

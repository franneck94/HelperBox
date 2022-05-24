#pragma once

#include <cstdint>
#include <string_view>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/GameContainers/GamePos.h>

#include <GuiUtils.h>
#include <Helper.h>
#include <Player.h>
#include <Skillbars.h>
#include <Types.h>
#include <Utils.h>

class ActionABC
{
public:
    ActionABC(Player *p, char *const t) : player(p), text(t)
    {
    }

    void Draw(const ImVec2 button_size = DEFAULT_BUTTON_SIZE);
    virtual RoutineState Routine() = 0;
    virtual void Update() = 0;

    Player *player = nullptr;
    ActionState action_state = ActionState::INACTIVE;
    char *const text;
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

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
    char *const text = nullptr;

    ActionState action_state = ActionState::INACTIVE;
};

class EmoActionABC : public ActionABC
{
public:
    EmoActionABC(Player *p, char *const t, EmoSkillbar *s) : ActionABC(p, t), skillbar(s)
    {
    }

    EmoSkillbar *skillbar = nullptr;
};

class MesmerActionABC : public ActionABC
{
public:
    MesmerActionABC(Player *p, char *const t, MesmerSkillbar *s) : ActionABC(p, t), skillbar(s)
    {
    }

    MesmerSkillbar *skillbar = nullptr;
};

RoutineState SafeWalk(GW::GamePos target_position, const bool reset = false);

RoutineState SafeUseSkill(const uint32_t skill_idx, const uint32_t target = 0, const uint32_t call_target = 0);

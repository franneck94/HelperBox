#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/GameContainers/GamePos.h>

#include <GuiConstants.h>
#include <Player.h>
#include <Skillbars.h>
#include <Types.h>

class ActionABC
{
public:
    ActionABC(Player *p, std::string_view t) : player(p), text(t)
    {
    }
    virtual ~ActionABC(){};

    void Draw(const ImVec2 button_size = DEFAULT_BUTTON_SIZE);
    virtual RoutineState Routine() = 0;
    virtual void Update() = 0;

    Player *player = nullptr;
    std::string_view text = nullptr;

    ActionState action_state = ActionState::INACTIVE;
};

class EmoActionABC : public ActionABC
{
public:
    EmoActionABC(Player *p, std::string_view t, EmoSkillbar *s) : ActionABC(p, t), skillbar(s)
    {
    }
    virtual ~EmoActionABC(){};

    EmoSkillbar *skillbar = nullptr;
};

class MesmerActionABC : public ActionABC
{
public:
    MesmerActionABC(Player *p, std::string_view t, MesmerSkillbar *s) : ActionABC(p, t), skillbar(s)
    {
    }
    virtual ~MesmerActionABC(){};

    MesmerSkillbar *skillbar = nullptr;
};

class DbActionABC : public ActionABC
{
public:
    DbActionABC(Player *p, std::string_view t, DbSkillbar *s) : ActionABC(p, t), skillbar(s)
    {
    }
    virtual ~DbActionABC(){};

    DbSkillbar *skillbar = nullptr;
};

bool HasWaitedLongEnough();

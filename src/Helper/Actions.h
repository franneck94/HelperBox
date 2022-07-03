#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/GameContainers/GamePos.h>

#include <GuiConstants.h>
#include <PlayerData.h>
#include <Skillbars.h>
#include <Types.h>

class ActionABC
{
public:
    ActionABC(PlayerData *p, std::string_view t) : player_data(p), text(t)
    {
    }
    virtual ~ActionABC(){};

    void Draw(const ImVec2 button_size = DEFAULT_BUTTON_SIZE);
    virtual RoutineState Routine() = 0;
    virtual void Update() = 0;

    virtual bool PauseRoutine()
    {
        return false;
    }
    bool ResumeRoutine()
    {
        return !PauseRoutine();
    }

    PlayerData *player_data = nullptr;
    std::string_view text = nullptr;

    ActionState action_state = ActionState::INACTIVE;
};

class EmoActionABC : public ActionABC
{
public:
    EmoActionABC(PlayerData *p, std::string_view t, EmoSkillbarData *s) : ActionABC(p, t), skillbar(s)
    {
    }
    virtual ~EmoActionABC(){};

    EmoSkillbarData *skillbar = nullptr;
};

class MesmerActionABC : public ActionABC
{
public:
    MesmerActionABC(PlayerData *p, std::string_view t, MesmerSkillbarData *s) : ActionABC(p, t), skillbar(s)
    {
    }
    virtual ~MesmerActionABC(){};

    MesmerSkillbarData *skillbar = nullptr;
};

class DbActionABC : public ActionABC
{
public:
    DbActionABC(PlayerData *p, std::string_view t, DbSkillbarData *s) : ActionABC(p, t), skillbar(s)
    {
    }
    virtual ~DbActionABC(){};

    DbSkillbarData *skillbar = nullptr;
};

bool HasWaitedLongEnough();

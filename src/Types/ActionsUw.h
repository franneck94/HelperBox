
#pragma once

#include <cstdint>
#include <string_view>

#include <Actions.h>
#include <PlayerData.h>
#include <SkillbarDataUw.h>

class EmoActionABC : public ActionABC
{
public:
    EmoActionABC(PlayerData *p, std::string_view t, EmoSkillbarData *s) : ActionABC(p, t), skillbar(s){};
    virtual ~EmoActionABC(){};

    EmoSkillbarData *skillbar = nullptr;
};

class MesmerActionABC : public ActionABC
{
public:
    MesmerActionABC(PlayerData *p, std::string_view t, MesmerSkillbarData *s) : ActionABC(p, t), skillbar(s){};
    virtual ~MesmerActionABC(){};

    MesmerSkillbarData *skillbar = nullptr;
};

class DbActionABC : public ActionABC
{
public:
    DbActionABC(PlayerData *p, std::string_view t, DbSkillbarData *s) : ActionABC(p, t), skillbar(s){};
    virtual ~DbActionABC(){};

    DbSkillbarData *skillbar = nullptr;
};
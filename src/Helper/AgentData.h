#pragma once

#include <functional>
#include <vector>

#include <PlayerData.h>

class AgentData
{
    AgentData(const PlayerData &_player)
        : player_data(_player), allies({}), neutrals({}), enemies({}), spirits({}), minions({}), npcs({}){};
    ~AgentData(){};

    bool ValidateData(std::function<bool()> cb_fn) const;
    void Update();

    void UpdateAllies();
    void UpdateNeutrals();
    void UpdateEnemies();
    void UpdateSpirits();
    void UpdateMinions();
    void UpdateNpcs();

private:
    PlayerData player_data;

    std::vector<GW::Agent *> allies;
    std::vector<GW::Agent *> neutrals;
    std::vector<GW::Agent *> enemies;
    std::vector<GW::Agent *> spirits;
    std::vector<GW::Agent *> minions;
    std::vector<GW::Agent *> npcs;
};

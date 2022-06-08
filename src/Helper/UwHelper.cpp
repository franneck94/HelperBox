
#include <cstdint>

#include <GWCA/Constants/Maps.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/MapMgr.h>

#include <Helper.h>
#include <MathUtils.h>
#include <Player.h>
#include <Types.h>

#include "UwHelper.h"

bool IsUw()
{
#ifdef _DEBUG
    return (GW::Map::GetMapID() == GW::Constants::MapID::The_Underworld ||
            GW::Map::GetMapID() == GW::Constants::MapID::Isle_of_the_Nameless);
#endif
    return (GW::Map::GetMapID() == GW::Constants::MapID::The_Underworld);
}

bool IsInVale(Player *player)
{
    if (!IsUw())
        return false;

    const auto pos1 = GW::GamePos{-13872.34F, 2332.34F, player->pos.zplane};
    const auto pos2 = GW::GamePos{-13760.19F, 358.15F, player->pos.zplane};

    const auto dist1 = GW::GetDistance(player->pos, pos1);
    const auto dist2 = GW::GetDistance(player->pos, pos2);

    if (dist1 < GW::Constants::Range::Spellcast || dist2 < GW::Constants::Range::Spellcast)
        return true;

    return false;
}

bool IsInDhuumRoom(const Player *const player)
{
    if (!player)
        return false;

    if (GW::Map::GetMapID() != GW::Constants::MapID::The_Underworld)
        return false;

    const auto dhuum_center_pos = GW::GamePos{-16105.50F, 17284.84F, player->pos.zplane};
    const auto dhuum_center_dist = GW::GetDistance(player->pos, dhuum_center_pos);

    if (dhuum_center_dist < GW::Constants::Range::Spellcast)
        return true;

    return false;
}

bool IsInDhuumFight(uint32_t *dhuum_id, float *dhuum_hp)
{
    if (GW::Map::GetMapID() != GW::Constants::MapID::The_Underworld)
        return false;

    const auto agents_array = GW::Agents::GetAgentArray();
    const GW::Agent *dhuum_agent = nullptr;

    for (const auto &agent : agents_array)
    {
        if (!agent)
            continue;

        const auto living = agent->GetAsAgentLiving();
        if (!living || living->allegiance != static_cast<uint8_t>(GW::Constants::Allegiance::Enemy))
            continue;

        if (living->player_number == static_cast<uint16_t>(GW::Constants::ModelID::UW::Dhuum))
        {
            dhuum_agent = agent;
            break;
        }
    }

    if (!dhuum_agent)
        return false;

    const auto dhuum_living = dhuum_agent->GetAsAgentLiving();
    if (!dhuum_living)
        return false;

    if (dhuum_living->allegiance == static_cast<uint8_t>(GW::Constants::Allegiance::Enemy))
    {
        *dhuum_id = dhuum_living->agent_id;
        *dhuum_hp = dhuum_living->hp;
        return true;
    }

    return false;
}

uint32_t GetClosestReaperID(Player &player)
{
    auto agents_array = GW::Agents::GetAgentArray();
    std::vector<GW::AgentLiving *> reapers;
    FilterAgents(player,
                 agents_array,
                 reapers,
                 std::array<uint32_t, 1>{GW::Constants::ModelID::UW::Reapers},
                 GW::Constants::Allegiance::Npc_Minipet,
                 GW::Constants::Range::Compass);

    if (reapers.size() == 0)
        return 0;

    return reapers[0]->agent_id;
}

bool TankIsFullteamLT()
{
    const uint32_t lt_id = GetTankId();
    if (!lt_id)
        return false;

    const auto lt_agent = GW::Agents::GetAgentByID(lt_id);

    if (!lt_agent)
        return false;

    const auto lt_living = lt_agent->GetAsAgentLiving();
    if (!lt_living)
        return false;

    if (lt_living->primary == static_cast<uint8_t>(GW::Constants::Profession::Mesmer) &&
        lt_living->secondary == static_cast<uint8_t>(GW::Constants::Profession::Assassin))
        return true;

    return false;
}

bool TankIsSoloLT()
{
    const uint32_t lt_id = GetTankId();
    if (!lt_id)
        return false;

    const auto lt_agent = GW::Agents::GetAgentByID(lt_id);

    if (!lt_agent)
        return false;

    const auto lt_living = lt_agent->GetAsAgentLiving();
    if (!lt_living)
        return false;

    if (lt_living->primary == static_cast<uint8_t>(GW::Constants::Profession::Mesmer) &&
        lt_living->secondary == static_cast<uint8_t>(GW::Constants::Profession::Elementalist))
        return true;

    return false;
}

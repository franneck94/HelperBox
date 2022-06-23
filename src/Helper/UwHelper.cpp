
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
    if (GW::Map::GetMapID() == GW::Constants::MapID::The_Underworld)
        return true;

#ifdef _DEBUG
    return (GW::Map::GetMapID() == GW::Constants::MapID::Isle_of_the_Nameless);
#else
    return false;
#endif
}

bool IsEmo(const Player &player)
{
    return (player.primary == GW::Constants::Profession::Elementalist &&
            player.secondary == GW::Constants::Profession::Monk);
}

bool IsDhuumBitch(const Player &player)
{
    return ((player.primary == GW::Constants::Profession::Ritualist ||
             player.primary == GW::Constants::Profession::Dervish) &&
            player.secondary == GW::Constants::Profession::Ranger);
}

bool IsSpiker(const Player &player)
{
    return (player.primary == GW::Constants::Profession::Mesmer &&
            player.secondary == GW::Constants::Profession::Ranger);
}

bool IsLT(const Player &player)
{
    return (player.primary == GW::Constants::Profession::Mesmer &&
            (player.secondary == GW::Constants::Profession::Elementalist ||
             player.secondary == GW::Constants::Profession::Assassin));
}

bool IsRangerTerra(const Player &player)
{
    return (player.primary == GW::Constants::Profession::Ranger &&
            player.secondary == GW::Constants::Profession::Assassin);
}

bool IsMesmerTerra(const Player &player)
{
    return (player.primary == GW::Constants::Profession::Mesmer &&
            player.secondary == GW::Constants::Profession::Elementalist);
}

bool IsAtSpirits1(const Player *const player)
{
    const auto pos = GW::GamePos{-13872.34F, 2332.34F, player->pos.zplane};
    const auto dist = GW::GetDistance(player->pos, pos);

    if (dist < GW::Constants::Range::Spellcast)
        return true;

    return false;
}

bool IsAtSpirits2(const Player *const player)
{
    const auto pos = GW::GamePos{-13760.19F, 358.15F, player->pos.zplane};
    const auto dist = GW::GetDistance(player->pos, pos);

    if (dist < GW::Constants::Range::Spellcast)
        return true;

    return false;
}

bool IsAtChamberSkele(const Player *const player)
{
    const auto pos = GW::GamePos{-2726.856F, 10239.48F, player->pos.zplane};
    const auto dist = GW::GetDistance(player->pos, pos);

    if (dist < GW::Constants::Range::Spellcast)
        return true;

    return false;
}

bool IsAtValeHouse(const Player *const player)
{
    const auto pos = GW::GamePos{-12264.12F, 1821.18F, 0};
    const auto dist = GW::GetDistance(player->pos, pos);

    if (dist < GW::Constants::Range::Spellcast)
        return true;

    return false;
}

bool IsSomewhereInVale(const Player *const player)
{
    const auto pos = GW::GamePos{-12264.12F, 1821.18F, 0};
    const auto dist = GW::GetDistance(player->pos, pos);

    if (dist < GW::Constants::Range::Spirit)
        return true;

    return false;
}

bool IsInVale(const Player *const player)
{
#ifdef _DEBUG
    if (GW::Map::GetMapID() == GW::Constants::MapID::Isle_of_the_Nameless)
        return true;
#endif

    if (!IsUw())
        return false;

    if (IsAtValeHouse(player) || IsAtSpirits1(player) || IsAtSpirits2(player))
        return true;

    return false;
}

bool IsAtFusePulls(const Player *const player)
{
    if (!IsUw())
        return false;

    const auto pos1 = GW::GamePos{-6263.33F, 9899.79F, player->pos.zplane}; // Fuse 1
    const auto pos2 = GW::GamePos{-7829.98F, 4324.09F, player->pos.zplane}; // Fuse 2

    const auto dist1 = GW::GetDistance(player->pos, pos1);
    const auto dist2 = GW::GetDistance(player->pos, pos2);

    if (dist1 < GW::Constants::Range::Spirit || dist2 < GW::Constants::Range::Spirit)
        return true;

    return false;
}

bool IsInDhuumRoom(const Player *const player)
{
    if (!IsUw())
        return false;

    const auto dhuum_center_pos = GW::GamePos{-16105.50F, 17284.84F, player->pos.zplane};
    const auto dhuum_center_dist = GW::GetDistance(player->pos, dhuum_center_pos);

    if (dhuum_center_dist < GW::Constants::Range::Spellcast)
        return true;

    return false;
}

bool IsInDhuumFight(uint32_t *dhuum_id, float *dhuum_hp)
{
    if (!IsUw())
        return false;

    const auto agents_array = GW::Agents::GetAgentArray();
    const GW::Agent *dhuum_agent = nullptr;

    for (const auto agent : agents_array)
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

bool TankIsFullteamLT()
{
    const auto lt_id = GetTankId();
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
    const auto lt_id = GetTankId();
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

bool TargetIsReaper(Player &player)
{
    if (!player.target)
        return false;

    const auto living_target = player.target->GetAsAgentLiving();
    if (!living_target || living_target->player_number != static_cast<uint32_t>(GW::Constants::ModelID::UW::Reapers))
        return false;

    return true;
}

void TargetClosestReaper(Player &player)
{
    TargetClosestNpcById(player, GW::Constants::ModelID::UW::Reapers);
}

void TalkClosestReaper(Player &player)
{
    const auto id = TargetClosestNpcById(player, GW::Constants::ModelID::UW::Reapers);

    if (!id)
        return;

    const auto agent = GW::Agents::GetAgentByID(id);
    if (!agent)
        return;

    GW::Agents::GoNPC(agent, 0);
}

void TargetClosestKeeper(Player &player)
{
    TargetClosestEnemyById(player, GW::Constants::ModelID::UW::KeeperOfSouls);
}

void AcceptChamber()
{
    const auto dialog = QuestRewardDialog(GW::Constants::QuestID::UW::Chamber);
    GW::Agents::SendDialog(dialog);
}

void TakeRestore()
{
    const auto dialog = QuestAcceptDialog(GW::Constants::QuestID::UW::Restore);
    GW::Agents::SendDialog(dialog);
}

void TakeEscort()
{
    const auto dialog = QuestAcceptDialog(GW::Constants::QuestID::UW::Escort);
    GW::Agents::SendDialog(dialog);
}

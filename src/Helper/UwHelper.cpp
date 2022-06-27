
#include <cstdint>

#include <GWCA/Constants/Maps.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/PartyMgr.h>

#include <Helper.h>
#include <MathUtils.h>
#include <Player.h>
#include <Types.h>

#include "UwHelper.h"

bool IsUw()
{
    return GW::Map::GetMapID() == GW::Constants::MapID::The_Underworld;
}

bool UwHelperActivationConditions()
{
    if (!HelperActivationConditions())
        return false;

    if (!IsUwEntryOutpost() && !IsUw())
        return false;

    return true;
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

bool IsAtSpawn(const Player &player)
{
    return IsNearToGamePos(player, GW::GamePos{1248.00F, 6965.51F, 0}, 500.0F);
}

bool IsAtChamberSkele(const Player &player)
{
    return IsNearToGamePos(player, GW::GamePos{-2726.856F, 10239.48F, 0}, 2000.0F);
}

bool IsAtBasementSkele(const Player &player)
{
    return IsNearToGamePos(player, GW::GamePos{-5183.64F, 8876.31F, 0}, 2000.0F);
}

bool IsRightAtChamberSkele(const Player &player)
{
    return IsNearToGamePos(player, GW::GamePos{-2726.856F, 10239.48F, 0}, 300.0F);
}

bool IsAtFusePull1(const Player &player)
{
    return IsNearToGamePos(player, GW::GamePos{-6263.33F, 9899.79F, 0}, 1500.0F);
}

bool IsAtFusePull2(const Player &player)
{
    return IsNearToGamePos(player, GW::GamePos{-7829.98F, 4324.09F, 0}, 1500.0F);
}

bool IsAtFusePulls(const Player &player)
{
    if (!IsUw())
        return false;

    if (IsAtFusePull1(player) || IsAtFusePull2(player))
        return true;

    return false;
}

bool IsAtValeStart(const Player &player)
{
    return IsNearToGamePos(player, GW::GamePos{-9764.08F, 2056.60F, 0}, 1500.0F);
}

bool IsAtValeHouse(const Player &player)
{
    return IsNearToGamePos(player, GW::GamePos{-12264.12F, 1821.18F, 0}, 1500.0F);
}

bool IsRightAtValeHouse(const Player &player)
{
    return IsNearToGamePos(player, GW::GamePos{-12264.12F, 1821.18F, 0}, 5.0F);
}

bool IsAtSpirits1(const Player &player)
{
    return IsNearToGamePos(player, GW::GamePos{-13872.34F, 2332.34F, 0}, GW::Constants::Range::Spellcast);
}

bool IsAtSpirits2(const Player &player)
{
    return IsNearToGamePos(player, GW::GamePos{-13760.19F, 358.15F, 0}, GW::Constants::Range::Spellcast);
}

bool IsAtValeSpirits(const Player &player)
{
    if (!IsUw())
        return false;

    if (IsAtSpirits1(player) || IsAtSpirits2(player))
        return true;

    return false;
}

bool IsInDhuumRoom(const Player &player)
{
#ifdef _DEBUG
    if (GW::Map::GetMapID() == GW::Constants::MapID::Isle_of_the_Nameless)
        return true;
#endif

    return IsNearToGamePos(player, GW::GamePos{-16105.50F, 17284.84F, 0}, GW::Constants::Range::Spellcast);
}

bool IsGoingToDhuum(const Player &player)
{
    return IsNearToGamePos(player, GW::GamePos{-3205.12F, 18812.15F, 0}, 500.0F);
}

bool IsInDhuumFight(uint32_t *dhuum_id, float *dhuum_hp, uint32_t *dhuum_max_hp)
{
#ifdef _DEBUG
    if (GW::Map::GetMapID() == GW::Constants::MapID::Isle_of_the_Nameless)
    {
        const auto me = GW::Agents::GetPlayer();
        if (!me || !me->agent_id)
            return false;
        const auto target = GW::Agents::GetTarget();
        if (!target)
            return false;
        const auto target_living = target->GetAsAgentLiving();
        if (!target_living)
            return false;
        if (dhuum_id)
            *dhuum_id = target_living->agent_id;
        if (dhuum_hp)
            *dhuum_hp = target_living->hp;
        if (dhuum_max_hp)
            *dhuum_max_hp = target_living->max_hp;
        return true;
    }
#endif

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
        if (dhuum_id)
            *dhuum_id = dhuum_living->agent_id;
        if (dhuum_hp)
            *dhuum_hp = dhuum_living->hp;
        if (dhuum_max_hp)
            *dhuum_max_hp = dhuum_living->max_hp;
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

bool TargetReaper(Player &player)
{
    TargetClosestNpcById(player, GW::Constants::ModelID::UW::Reapers);

    return true;
}

bool TalkReaper(Player &player)
{
    const auto id = TargetClosestNpcById(player, GW::Constants::ModelID::UW::Reapers);

    if (!id)
        return true;

    const auto agent = GW::Agents::GetAgentByID(id);
    if (!agent)
        return true;

    GW::Agents::GoNPC(agent, 0);

    return true;
}

bool TargetClosestKeeper(Player &player)
{
    TargetClosestEnemyById(player, GW::Constants::ModelID::UW::KeeperOfSouls);

    return true;
}

bool AcceptChamber()
{
    const auto dialog = QuestRewardDialog(GW::Constants::QuestID::UW::Chamber);
    GW::Agents::SendDialog(dialog);
    return true;
}

bool TakeRestore()
{
    const auto dialog = QuestAcceptDialog(GW::Constants::QuestID::UW::Restore);
    GW::Agents::SendDialog(dialog);
    return true;
}

bool TakeEscort()
{
    const auto dialog = QuestAcceptDialog(GW::Constants::QuestID::UW::Escort);
    GW::Agents::SendDialog(dialog);
    return true;
}

bool TakeUWG()
{
    const auto dialog = QuestAcceptDialog(GW::Constants::QuestID::UW::UWG);
    GW::Agents::SendDialog(dialog);
    return true;
}

bool TakePits()
{
    const auto dialog = QuestAcceptDialog(GW::Constants::QuestID::UW::Pits);
    GW::Agents::SendDialog(dialog);
    return true;
}

bool TakePlanes()
{
    const auto dialog = QuestAcceptDialog(GW::Constants::QuestID::UW::Planes);
    GW::Agents::SendDialog(dialog);
    return true;
}

bool FoundKeeperAtPos(const std::vector<GW::AgentLiving *> &keeper_livings, const GW::GamePos &keeper_pos)
{
    auto found_keeper = false;

    for (const auto keeper : keeper_livings)
    {
        if (GW::GetDistance(keeper->pos, keeper_pos) < GW::Constants::Range::Earshot)
        {
            found_keeper = true;
            break;
        }
    }

    return found_keeper;
}

bool DhuumIsCastingJudgement(const uint32_t dhuum_id)
{
    const auto dhuum_agent = GW::Agents::GetAgentByID(dhuum_id);
    if (!dhuum_agent)
        return false;

    const auto dhuum_living = dhuum_agent->GetAsAgentLiving();
    if (!dhuum_living)
        return false;

    if (dhuum_living->GetIsCasting() && dhuum_living->skill == static_cast<uint32_t>(3085))
        return true;

    return false;
}

#include <GWCA/Constants/Constants.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/Managers/MapMgr.h>

#include <HelperMaps.h>
#include <MathUtils.h>

#include "HelperUwPos.h"

bool IsUw()
{
    return GW::Map::GetMapID() == GW::Constants::MapID::The_Underworld;
}

bool IsUwEntryOutpost()
{
    return IsEndGameEntryOutpost();
}

bool IsAtSpawn(const GW::GamePos &player_pos)
{
    return IsNearToGamePos(player_pos, GW::GamePos{1248.00F, 6965.51F, 0}, 500.0F);
}

bool IsAtChamberSkele(const GW::GamePos &player_pos)
{
    return IsNearToGamePos(player_pos, GW::GamePos{-2726.856F, 10239.48F, 0}, 2250.0F);
}

bool IsAtBasementSkele(const GW::GamePos &player_pos)
{
    return IsNearToGamePos(player_pos, GW::GamePos{-5183.64F, 8876.31F, 0}, 2000.0F);
}

bool IsRightAtChamberSkele(const GW::GamePos &player_pos)
{
    return IsNearToGamePos(player_pos, GW::GamePos{-2726.856F, 10239.48F, 0}, 300.0F);
}

bool IsAtFusePull1(const GW::GamePos &player_pos)
{
    return IsNearToGamePos(player_pos, GW::GamePos{-6263.33F, 9899.79F, 0}, 1500.0F);
}

bool IsAtFusePull2(const GW::GamePos &player_pos)
{
    return IsNearToGamePos(player_pos, GW::GamePos{-7829.98F, 4324.09F, 0}, 1500.0F);
}

bool IsAtFusePulls(const GW::GamePos &player_pos)
{
    if (!IsUw())
        return false;

    if (IsAtFusePull1(player_pos) || IsAtFusePull2(player_pos))
        return true;

    return false;
}

bool IsAtValeStart(const GW::GamePos &player_pos)
{
    return IsNearToGamePos(player_pos, GW::GamePos{-9764.08F, 2056.60F, 0}, 1500.0F);
}

bool IsAtValeHouse(const GW::GamePos &player_pos)
{
    return IsNearToGamePos(player_pos, GW::GamePos{-12264.12F, 1821.18F, 0}, 1500.0F);
}

bool IsRightAtValeHouse(const GW::GamePos &player_pos)
{
    return IsNearToGamePos(player_pos, GW::GamePos{-12264.12F, 1821.18F, 0}, 5.0F);
}

bool IsAtSpirits1(const GW::GamePos &player_pos)
{
    return IsNearToGamePos(player_pos, GW::GamePos{-13872.34F, 2332.34F, 0}, GW::Constants::Range::Spellcast);
}

bool IsAtSpirits2(const GW::GamePos &player_pos)
{
    return IsNearToGamePos(player_pos, GW::GamePos{-13760.19F, 358.15F, 0}, GW::Constants::Range::Spellcast);
}

bool IsAtValeSpirits(const GW::GamePos &player_pos)
{
    if (!IsUw())
        return false;

    if (IsAtSpirits1(player_pos) || IsAtSpirits2(player_pos))
        return true;

    return false;
}

bool IsInDhuumRoom(const GW::GamePos &player_pos)
{
    return IsNearToGamePos(player_pos, GW::GamePos{-16105.50F, 17284.84F, 0}, GW::Constants::Range::Spellcast);
}

bool IsGoingToDhuum(const GW::GamePos &player_pos)
{
    return IsNearToGamePos(player_pos, GW::GamePos{-9567.56F, 17288.916F, 0}, 100.0F);
}

bool IsAtFilterSkelePos(const GW::GamePos &player_pos, const GW::GamePos &next_pos)
{
    const auto is_near_chamber_skele = IsAtChamberSkele(player_pos);
    const auto is_right_at_chamber_skele = IsRightAtChamberSkele(player_pos);
    const auto is_in_chamber_where_to_move = (is_near_chamber_skele && !is_right_at_chamber_skele);

    const auto is_near_to_at_vale_start = IsAtValeStart(player_pos);
    const auto is_near_to_vale_house = IsAtValeHouse(player_pos);
    const auto is_right_at_vale_house = IsRightAtValeHouse(player_pos);
    const auto is_in_vale_where_to_move =
        ((is_near_to_at_vale_start || is_near_to_vale_house) && !is_right_at_vale_house);

    const auto is_at_basement_stair = GW::GetDistance(next_pos, GW::GamePos{-6263.33F, 9899.79F, 0}) < 1280.0F;
    const auto is_to_basement1 = GW::GetDistance(next_pos, GW::GamePos{-5183.64F, 8876.31F, 0}) < 1280.0F;

    return (is_in_chamber_where_to_move || is_in_vale_where_to_move || is_to_basement1 || is_at_basement_stair);
}
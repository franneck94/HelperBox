#pragma once

#include <vector>

#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Agent.h>

bool FloatCompare(const float a, const float b, const float epsilon = 1e-3F);

bool GamePosCompare(const GW::GamePos &p1, const GW::GamePos &p2, const float epsilon = 1e-3F);

GW::GamePos MovePointAlongVector(const GW::GamePos &pos1, const GW::GamePos &pos2, const float move_amount);

class GameRectangle
{
public:
    GameRectangle(const GW::GamePos &p1, const GW::GamePos &p2, const float offset);

    bool PointInGameRectangle(const GW::GamePos &pt) const;

    static bool PointInTriangle(const GW::GamePos &pt,
                                const GW::GamePos &v1,
                                const GW::GamePos &v2,
                                const GW::GamePos &v3);
    static float Sign(const GW::GamePos &p1, const GW::GamePos &p2, const GW::GamePos &p3);

public:
    GW::GamePos v1;
    GW::GamePos v2;
    GW::GamePos v3;
    GW::GamePos v4;
};

template <typename T>
constexpr T PI = T(3.14159265358979323846L);

template <typename T>
[[nodiscard]] constexpr T deg_to_rad(const T deg)
{
    static_assert(std::is_floating_point_v<T>, "Must be floating point type.");

    return deg * (PI<T> / static_cast<T>(180.0));
}

GW::GamePos RotatePoint(const GW::GamePos &player_pos, GW::GamePos pos, const float theta, const bool swap = true);

bool IsNearToGamePos(const GW::GamePos &player_pos, const GW::GamePos &pos, const float r);

std::vector<GW::AgentLiving *> GetEnemiesInGameRectangle(const GameRectangle &rectangle,
                                                         const std::vector<GW::AgentLiving *> &living_agents);

bool PointIsBelowLine(const float slope, const float bias, const GW::GamePos &point);

std::pair<float, float> GetLineBasedOnPointAndAngle(const GW::GamePos &player_pos, const float theta);
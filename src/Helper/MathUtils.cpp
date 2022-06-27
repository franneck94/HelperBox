#include <cmath>

#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Camera.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/CameraMgr.h>

#include <Player.h>

#include "MathUtils.h"

bool FloatCompare(const float a, const float b, const float epsilon)
{
    return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

bool GamePosCompare(const GW::GamePos &p1, const GW::GamePos &p2, const float epsilon)
{
    return (FloatCompare(p1.x, p2.x, epsilon) && FloatCompare(p1.y, p2.y, epsilon));
}

GW::GamePos MovePointAlongVector(const GW::GamePos &pos1, const GW::GamePos &pos2, const float move_amount)
{
    const auto dist = GW::GetNorm(GW::Vec2f{pos1.x - pos2.x, pos1.y - pos2.y});
    const auto d_t = dist + move_amount;
    const auto t = d_t / dist;

    const auto p_x = ((1.0F - t) * pos2.x + t * pos1.x);
    const auto p_y = ((1.0F - t) * pos2.y + t * pos1.y);

    return GW::GamePos{p_x, p_y, 0};
}

GameRectangle::GameRectangle(const GW::GamePos &p1, const GW::GamePos &p2, const float offset)
{
    const auto adj_p1 = MovePointAlongVector(p1, p2, offset * 0.10F); // Behind player
    const auto adj_p2 = MovePointAlongVector(p2, p1, offset * 1.20F); // In front of Target

    const auto delta_x = adj_p1.x - adj_p2.x;
    const auto delta_y = adj_p1.y - adj_p2.y;
    const auto dist = GW::GetDistance(adj_p1, adj_p2);

    const auto half_offset = offset * 0.92F; // To the side

    v1 = GW::GamePos{adj_p1.x + ((-delta_y) / dist) * half_offset, adj_p1.y + (delta_x / dist) * half_offset, 0};
    v2 = GW::GamePos{adj_p1.x + ((-delta_y) / dist) * (-half_offset), adj_p1.y + (delta_x / dist) * (-half_offset), 0};
    v3 = GW::GamePos{adj_p2.x - (delta_y / dist) * half_offset, adj_p2.y + (delta_x / dist) * half_offset, 0};
    v4 = GW::GamePos{adj_p2.x + ((-delta_y) / dist) * (-half_offset), adj_p2.y + (delta_x / dist) * (-half_offset), 0};
}

float GameRectangle::Sign(const GW::GamePos &p1, const GW::GamePos &p2, const GW::GamePos &p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool GameRectangle::PointInGameRectangle(const GW::GamePos &pt) const
{
    return PointInTriangle(pt, v1, v2, v3) || PointInTriangle(pt, v4, v2, v3);
}

bool GameRectangle::PointInTriangle(const GW::GamePos &pt,
                                    const GW::GamePos &v1,
                                    const GW::GamePos &v2,
                                    const GW::GamePos &v3)
{
    const auto b1 = Sign(pt, v1, v2) < 0.0f;
    const auto b2 = Sign(pt, v2, v3) < 0.0f;
    const auto b3 = Sign(pt, v3, v1) < 0.0f;

    return ((b1 == b2) && (b2 == b3));
}

GW::GamePos rotate_point(const Player &player, GW::GamePos pos)
{
    GW::GamePos v(pos.x, pos.y, 0);

    v.x = pos.x - player.pos.x;
    v.y = player.pos.y - pos.y;

    const auto angle = (GW::CameraMgr::GetCamera()->GetCurrentYaw() + static_cast<float>(M_PI_2));
    const auto x1 = v.x * std::cos(angle) - v.y * std::sin(angle);
    const auto y1 = v.x * std::sin(angle) + v.y * std::cos(angle);
    v = GW::GamePos(x1, y1, 0);

    return v;
}

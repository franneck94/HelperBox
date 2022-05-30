#include "Utils.h"

bool FloatCompare(const float a, const float b, const float epsilon)
{
    return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

bool GamePosCompare(const GW::GamePos &p1, const GW::GamePos &p2, const float epsilon)
{
    return (FloatCompare(p1.x, p2.x, epsilon) && FloatCompare(p1.y, p2.y, epsilon));
}

#pragma once

#include <cmath>
#include <cstdint>
#include <random>
#include <type_traits>
#include <utility>

#include <GWCA/GameContainers/GamePos.h>

#include "Timer.h"

template <typename T>
T RandomInt(T lower, T upper)
{
    static_assert(std::is_integral_v<T>, "Must be integral type.");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<T> dist(lower, upper);

    return dist(gen);
}

template <typename T>
T RandomFloat(T lower, T upper)
{
    static_assert(std::is_floating_point_v<T>, "Must be floating point type.");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<T> dist(lower, upper);

    return dist(gen);
}

template <typename T>
bool FloatCompare(const T a, const T b, const T epsilon = 1e-3F)
{
    static_assert(std::is_floating_point_v<T>, "Must be floating point type.");

    return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

template <typename T>
bool GamePosCompare(const GW::GamePos &p1, const GW::GamePos &p2, const T epsilon = 1e-3F)
{
    static_assert(std::is_floating_point_v<T>, "Must be floating point type.");

    return (FloatCompare(p1.x, p2.x, epsilon) && FloatCompare(p1.y, p2.y, epsilon));
}

template <typename T>
void Wait(const T wait_time_ms)
{
    static_assert(std::is_integral_v<T>, "Must be integral type.");

    const auto timer = TIMER_INIT();

    while (true)
    {
        const auto diff = TIMER_DIFF(timer);

        if (diff > wait_time_ms)
        {
            return;
        }
    }
}

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

bool FloatCompare(const float a, const float b, const float epsilon = 1e-3F);

bool GamePosCompare(const GW::GamePos &p1, const GW::GamePos &p2, const float epsilon = 1e-3F);

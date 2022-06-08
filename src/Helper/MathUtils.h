#pragma once

#include <GWCA/GameContainers/GamePos.h>

bool FloatCompare(const float a, const float b, const float epsilon = 1e-3F);

bool GamePosCompare(const GW::GamePos &p1, const GW::GamePos &p2, const float epsilon = 1e-3F);

float Lerp(const float a, const float b, const float f);

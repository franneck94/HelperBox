#pragma once

#include <cstdint>

struct Settings
{
    uint32_t pid;
};

extern Settings settings;

void ParseCommandLine();

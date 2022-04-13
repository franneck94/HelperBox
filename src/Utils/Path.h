#pragma once

#include <filesystem>
#include <string_view>

std::filesystem::path GetSettingsFolderPath();

std::filesystem::path GetPath(std::wstring_view filename);

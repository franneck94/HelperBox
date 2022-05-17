#include <cstdlib>
#include <exception>
#include <iostream>
#include <fstream>

#include <Logger.h>

#include "Path.h"

std::filesystem::path GetSettingsFolderPath()
{
    const auto homepath = std::getenv("USERPROFILE");
    auto helperbox_settings_path = std::filesystem::path(homepath);
    helperbox_settings_path /= "helperbox";

    if (!std::filesystem::exists(helperbox_settings_path))
    {
        try
        {
            std::filesystem::create_directory(helperbox_settings_path);
        }
        catch (const std::exception &e)
        {
            Log::Error("%s\n", e.what());
        }
    }

    return helperbox_settings_path;
}

std::filesystem::path GetPath(std::wstring_view filename)
{
    const auto filepath = GetSettingsFolderPath() / filename;

    if (!std::filesystem::exists(filepath))
    {
        std::ofstream ofs(filepath);
        ofs.close();
    }

    return filepath;
}
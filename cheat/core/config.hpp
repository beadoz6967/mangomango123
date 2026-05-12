#pragma once

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace Config
{
    // Config file path
    extern fs::path g_ConfigPath;

    // Initialize config directory
    bool Init();

    // Save current settings to config
    bool Save();

    // Load settings from config
    bool Load();

    // Reset to defaults
    void ResetDefaults();
}

// core/config.cpp — fixed: Load() actually parses the INI now
#include "../pch.h"
#include "config.hpp"
#include "../gui/gui.h"
#include <shlobj.h>
#include <fstream>
#include <string>
#include <unordered_map>

namespace Config {
    fs::path g_ConfigPath;

    bool Init() {
        PWSTR appData = nullptr;
        if (FAILED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appData))) return false;
        g_ConfigPath = appData;
        CoTaskMemFree(appData);
        g_ConfigPath /= L"litware";
        if (!fs::exists(g_ConfigPath)) {
            try { fs::create_directories(g_ConfigPath); }
            catch (...) { return false; }
        }
        g_ConfigPath /= L"default.cfg";
        return true;
    }

    bool Save() {
        try {
            std::ofstream f(g_ConfigPath);
            if (!f.is_open()) return false;
            f << "[ESP]\nbox="         << GUI::bBoxESP        << "\nhealth="     << GUI::bHealthBar
              << "\nglow="             << GUI::bGlowESP        << "\nwallhack="   << GUI::bWallhackESP
              << "\nnames="            << GUI::bPlayerNames    << "\nweapons="    << GUI::bWeaponInfo
              << "\ndistance="         << GUI::bDistance       << "\narrows="     << GUI::bArrows
              << "\n[Aimbot]\nenabled="<< GUI::bAimbot         << "\naim_fov="    << GUI::fAimFOV
              << "\nsmooth="           << GUI::fAimSmooth      << "\nbone="       << GUI::iAimbotBone
              << "\nteamcheck="        << GUI::bAimbotTeamCheck<< "\n[Visuals]\nno_flash=" << GUI::bNoFlash
              << "\nno_smoke="         << GUI::bNoSmoke        << "\nsky_color="  << GUI::bSkyColor
              << "\nvis_fov="          << GUI::fFOVChanger
              << "\n[Movement]\nbhop=" << GUI::bBhop           << "\nstrafe="     << GUI::bStrafe
              << "\n[Misc]\nradar="    << GUI::bRadar          << "\nbomb_timer=" << GUI::bBombTimer
              << "\nspectator_list="   << GUI::bSpectatorList  << "\ntriggerbot=" << GUI::bTriggerbot << "\n";
            return true;
        } catch (...) { return false; }
    }

    // FIXED: actually parses key=value pairs into GUI state
    bool Load() {
        if (!fs::exists(g_ConfigPath)) return Save();

        std::ifstream f(g_ConfigPath);
        if (!f.is_open()) return false;

        std::unordered_map<std::string, std::string> kv;
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty() || line[0] == '[') continue;
            auto eq = line.find('=');
            if (eq == std::string::npos) continue;
            kv[line.substr(0, eq)] = line.substr(eq + 1);
        }

        auto getB = [&](const char* k, bool def) -> bool {
            auto it = kv.find(k); return it != kv.end() ? (it->second == "1") : def;
        };
        auto getF = [&](const char* k, float def) -> float {
            auto it = kv.find(k);
            if (it == kv.end()) return def;
            try { return std::stof(it->second); } catch (...) { return def; }
        };
        auto getI = [&](const char* k, int def) -> int {
            auto it = kv.find(k);
            if (it == kv.end()) return def;
            try { return std::stoi(it->second); } catch (...) { return def; }
        };

        GUI::bBoxESP          = getB("box",            true);
        GUI::bHealthBar       = getB("health",         true);
        GUI::bGlowESP         = getB("glow",           false);
        GUI::bWallhackESP     = getB("wallhack",       false);
        GUI::bPlayerNames     = getB("names",          true);
        GUI::bWeaponInfo      = getB("weapons",        true);
        GUI::bDistance        = getB("distance",       true);
        GUI::bArrows          = getB("arrows",         false);
        GUI::bAimbot          = getB("enabled",        false);
        GUI::fAimFOV          = getF("aim_fov",        5.0f);
        GUI::fAimSmooth       = getF("smooth",         3.0f);
        GUI::iAimbotBone      = getI("bone",           1);
        GUI::bAimbotTeamCheck = getB("teamcheck",      true);
        GUI::bNoFlash         = getB("no_flash",       false);
        GUI::bNoSmoke         = getB("no_smoke",       false);
        GUI::bSkyColor        = getB("sky_color",      false);
        GUI::fFOVChanger      = getF("vis_fov",        90.0f);
        GUI::bBhop            = getB("bhop",           false);
        GUI::bStrafe          = getB("strafe",         false);
        GUI::bRadar           = getB("radar",          false);
        GUI::bBombTimer       = getB("bomb_timer",     false);
        GUI::bSpectatorList   = getB("spectator_list", false);
        GUI::bTriggerbot      = getB("triggerbot",     false);

        return true;
    }

    void ResetDefaults() {
        GUI::bBoxESP = true; GUI::bHealthBar = true; GUI::bGlowESP = false;
        GUI::bWallhackESP = false; GUI::bPlayerNames = true; GUI::bWeaponInfo = true;
        GUI::bDistance = true; GUI::bArrows = false; GUI::bAimbot = false;
        GUI::fAimFOV = 5.f; GUI::fAimSmooth = 3.f; GUI::iAimbotBone = 1;
        GUI::bAimbotTeamCheck = true; GUI::bNoFlash = false; GUI::bNoSmoke = false;
        GUI::bSkyColor = false; GUI::fFOVChanger = 90.f; GUI::bBhop = false;
        GUI::bStrafe = false; GUI::bRadar = false; GUI::bBombTimer = false;
        GUI::bSpectatorList = false; GUI::bTriggerbot = false;
        Save();
    }
}

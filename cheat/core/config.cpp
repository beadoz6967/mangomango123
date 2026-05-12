#include "../pch.h"
#include "config.hpp"
#include "../gui/gui.h"
#include <shlobj.h>
#include <fstream>

namespace Config {
    fs::path g_ConfigPath;

    bool Init() {
        PWSTR appDataPath = nullptr;
        if (FAILED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataPath)))
            return false;
        g_ConfigPath = appDataPath;
        CoTaskMemFree(appDataPath);
        g_ConfigPath /= L"litware";
        if (!fs::exists(g_ConfigPath)) {
            try { fs::create_directories(g_ConfigPath); }
            catch (const fs::filesystem_error&) { return false; }
        }
        g_ConfigPath /= L"default.cfg";
        return true;
    }

    bool Save() {
        try {
            std::ofstream file(g_ConfigPath);
            if (!file.is_open()) return false;
            file << "[ESP]\nbox=" << GUI::bBoxESP << "\nhealth=" << GUI::bHealthBar << "\nglow=" << GUI::bGlowESP << "\n";
            file << "wallhack=" << GUI::bWallhackESP << "\nnames=" << GUI::bPlayerNames << "\nweapons=" << GUI::bWeaponInfo << "\n";
            file << "distance=" << GUI::bDistance << "\narrows=" << GUI::bArrows << "\n";
            file << "[Aimbot]\nenabled=" << GUI::bAimbot << "\nfov=" << GUI::fAimFOV << "\nsmooth=" << GUI::fAimSmooth << "\n";
            file << "bone=" << GUI::iAimbotBone << "\nteamcheck=" << GUI::bAimbotTeamCheck << "\n";
            file << "[Visuals]\nno_flash=" << GUI::bNoFlash << "\nno_smoke=" << GUI::bNoSmoke << "\n";
            file << "sky_color=" << GUI::bSkyColor << "\nfov=" << GUI::fFOVChanger << "\n";
            file << "[Movement]\nbhop=" << GUI::bBhop << "\nstrafe=" << GUI::bStrafe << "\n";
            file << "[Misc]\nradar=" << GUI::bRadar << "\nbomb_timer=" << GUI::bBombTimer << "\n";
            file << "spectator_list=" << GUI::bSpectatorList << "\ntriggerbot=" << GUI::bTriggerbot << "\n";
            file.close();
            return true;
        } catch (...) { return false; }
    }

    bool Load() {
        if (!fs::exists(g_ConfigPath)) return Save();
        return true;
    }

    void ResetDefaults() {
        GUI::bBoxESP = true;
        GUI::bHealthBar = true;
        GUI::bGlowESP = false;
        GUI::bWallhackESP = false;
        GUI::bPlayerNames = true;
        GUI::bWeaponInfo = true;
        GUI::bDistance = true;
        GUI::bArrows = false;
        GUI::bAimbot = false;
        GUI::fAimFOV = 5.0f;
        GUI::fAimSmooth = 3.0f;
        GUI::iAimbotBone = 1;
        GUI::bAimbotTeamCheck = true;
        GUI::bNoFlash = false;
        GUI::bNoSmoke = false;
        GUI::bSkyColor = false;
        GUI::fFOVChanger = 90.0f;
        GUI::bBhop = false;
        GUI::bStrafe = false;
        GUI::bRadar = false;
        GUI::bBombTimer = false;
        GUI::bSpectatorList = false;
        GUI::bTriggerbot = false;
        Save();
    }
}

#pragma once

namespace GUI
{
    inline bool g_Open = false;

    // ESP
    inline bool  bBoxESP           = true;
    inline bool  bCornerBox        = false;   // true = L-corners only, false = full rect
    inline bool  bSnaplines        = false;
    inline bool  bHealthBar        = true;
    inline bool  bHealthNumber     = false;   // show HP value on health bar
    inline bool  bArmorBar         = false;   // blue bar, right side of box
    inline bool  bGlowESP          = false;
    inline bool  bWallhackESP      = false;
    inline bool  bPlayerNames      = true;
    inline bool  bWeaponName       = true;    // weapon name string below box
    inline bool  bWeaponInfo       = true;    // clip/reserve numbers
    inline bool  bEspFlags         = true;    // [HE][KIT][C4] flags
    inline bool  bDistance         = true;
    inline bool  bArrows           = false;

    // Aimbot
    inline bool  bAimbot           = false;
    inline float fAimFOV           = 5.0f;
    inline float fAimSmooth        = 3.0f;
    inline int   iAimbotBone       = 1;  // 0 = body, 1 = head
    inline bool  bAimbotTeamCheck  = true;

    // Visuals
    inline bool  bNoFlash          = false;
    inline bool  bNoSmoke          = false;
    inline bool  bSkyColor         = false;
    inline float fFOVChanger       = 90.0f;

    // Movement
    inline bool  bBhop             = false;
    inline bool  bStrafe           = false;

    // Misc
    inline bool  bRadar            = false;
    inline bool  bBombTimer        = false;
    inline bool  bSpectatorList    = false;
    inline bool  bTriggerbot       = false;

    void ApplyStyle();
    void Render();
}

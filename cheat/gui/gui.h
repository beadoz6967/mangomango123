#pragma once

namespace GUI
{
    inline bool g_Open = false;

    inline bool  bBhop       = false;
    inline bool  bGlowESP    = false;
    inline bool  bTriggerbot = false;
    inline float fAimFOV     = 5.0f;
    inline float fAimSmooth  = 3.0f;
    inline bool bBoxESP    = false;
    inline bool bHealthBar = false;

    void ApplyStyle();
    void Render();
}

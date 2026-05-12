#include "../pch.h"
#include "wallhack.hpp"
#include "../imgui/imgui.h"

namespace Wallhack {
    void RenderWallhack(ImDrawList* dl, float boxX, float boxY, float boxW, float boxH, 
                       unsigned int col, float visibility, float baseAlpha) {
        if (!dl) return;

        int r = (col >> IM_COL32_R_SHIFT) & 0xFF;
        int g = (col >> IM_COL32_G_SHIFT) & 0xFF;
        int b = (col >> IM_COL32_B_SHIFT) & 0xFF;

        float wallhackAlpha = baseAlpha * visibility;

        if (visibility < 0.99f) {
            unsigned int wallhackCol = IM_COL32(r, g, b, (int)(wallhackAlpha * 255 * 0.4f));
            dl->AddRect({ boxX - 1, boxY - 1 }, { boxX + boxW + 1, boxY + boxH + 1 }, 
                       IM_COL32(0, 0, 0, (int)(80 * visibility)), 0.0f, 0, 1.5f);
            dl->AddRect({ boxX, boxY }, { boxX + boxW, boxY + boxH }, wallhackCol, 
                       0.0f, 0, 1.2f);
        }
    }
}

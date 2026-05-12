#include "../pch.h"
#include "glow.hpp"
#include "../imgui/imgui.h"

namespace Glow {
    void RenderGlow(ImDrawList* dl, float boxX, float boxY, float boxW, float boxH, 
                   unsigned int col, float alpha) {
        if (!dl || boxW <= 0.f || boxH <= 0.f) return;

        int r = (col >> IM_COL32_R_SHIFT) & 0xFF;
        int g = (col >> IM_COL32_G_SHIFT) & 0xFF;
        int b = (col >> IM_COL32_B_SHIFT) & 0xFF;

        for (int glowLayer = 6; glowLayer >= 1; glowLayer--) {
            float offset = (float)glowLayer;
            int glowAlpha = (int)(45.0f * alpha / (float)glowLayer);

            unsigned int glowCol = IM_COL32(r, g, b, glowAlpha);

            dl->AddRect(
                { boxX - offset - 1.f, boxY - offset - 1.f },
                { boxX + boxW + offset + 1.f, boxY + boxH + offset + 1.f },
                glowCol,
                0.0f, 0, 0.8f
            );
        }

        unsigned int mainCol = IM_COL32(r, g, b, (int)(alpha * 255));
        dl->AddRect(
            { boxX, boxY },
            { boxX + boxW, boxY + boxH },
            mainCol,
            0.0f, 0, 1.2f
        );
    }
}

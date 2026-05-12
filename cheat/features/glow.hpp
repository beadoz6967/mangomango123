#pragma once

struct ImDrawList;

namespace Glow {
    void RenderGlow(ImDrawList* dl, float boxX, float boxY, float boxW, float boxH, 
                   unsigned int col, float alpha);
}

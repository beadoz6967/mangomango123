#pragma once

class ImDrawList;

namespace Wallhack {
    void RenderWallhack(ImDrawList* dl, float boxX, float boxY, float boxW, float boxH, 
                       unsigned int col, float visibility, float baseAlpha);
}

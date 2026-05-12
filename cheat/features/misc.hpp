#pragma once

struct ImDrawList;

namespace Radar
{
    void Render(ImDrawList* dl, float screenWidth, float screenHeight);
}

namespace Misc
{
    void Update();
}

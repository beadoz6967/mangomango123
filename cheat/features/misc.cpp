#include "../pch.h"
#include "misc.hpp"
#include "../gui/gui.h"
#include "../sdk/offsets.hpp"
#include "../imgui/imgui.h"
#include <Windows.h>
#include <cmath>

using namespace cs2_dumper::offsets;

struct Vec2 { float x, y; Vec2 operator+(const Vec2& o) const { return {x+o.x, y+o.y}; } Vec2 operator*(float s) const { return {x*s, y*s}; } float Length() const { return std::sqrt(x*x + y*y); } };
struct Vec3 { float x, y, z; Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; } };

template<typename T>
static T MiscRd(uintptr_t addr) {
    T r{};
    if (addr < 0x100000 || addr > 0x7FFFFFFEFFFF) return r;
    __try { r = *reinterpret_cast<T*>(addr); } __except(EXCEPTION_EXECUTE_HANDLER) { return T{}; }
    return r;
}

namespace PawnOffsetsMisc {
    constexpr ptrdiff_t m_iTeamNum       = 0x3E3;
    constexpr ptrdiff_t m_lifeState      = 0x348;
    constexpr ptrdiff_t m_pGameSceneNode = 0x200;
}
namespace CtrlOffsetsMisc { constexpr ptrdiff_t m_hPlayerPawn = 0x608; }
namespace NodeOffsetsMisc  { constexpr ptrdiff_t m_vRenderOrigin = 0x274; }

void Radar::Render(ImDrawList* dl, float screenWidth, float screenHeight)
{
    if (!dl || screenWidth < 100.f || screenHeight < 100.f) return;

    const auto clientBase = reinterpret_cast<uintptr_t>(GetModuleHandleA("client.dll"));
    if (!clientBase) return;

    const uintptr_t entList   = MiscRd<uintptr_t>(clientBase + client_dll::dwEntityList);
    const uintptr_t localPawn = MiscRd<uintptr_t>(clientBase + client_dll::dwLocalPlayerPawn);

    if (!entList || !localPawn) return;

    const uintptr_t localNode = MiscRd<uintptr_t>(localPawn + PawnOffsetsMisc::m_pGameSceneNode);
    const Vec3 localPos = localNode ? MiscRd<Vec3>(localNode + NodeOffsetsMisc::m_vRenderOrigin) : Vec3{};

    float mapScale = 0.08f;
    ImVec2 radarCenter = { screenWidth * 0.5f, screenHeight * 0.5f };

    // Draw radar background
    dl->AddRectFilled({ radarCenter.x - 80.f, radarCenter.y - 80.f }, 
                     { radarCenter.x + 80.f, radarCenter.y + 80.f }, 
                     IM_COL32(20, 20, 30, 200));
    dl->AddRect({ radarCenter.x - 80.f, radarCenter.y - 80.f }, 
               { radarCenter.x + 80.f, radarCenter.y + 80.f }, 
               IM_COL32(100, 100, 150, 255), 1.0f);

    // Draw local player at center
    dl->AddCircleFilled(radarCenter, 3.f, IM_COL32(0, 255, 0, 255), 8);

    int radarCount = 0;
    for (int i = 1; i < 128; i++) {
        const uintptr_t chunk = MiscRd<uintptr_t>(entList + 16 + 8 * ((i & 0x7FFF) >> 9));
        if (!chunk) continue;
        const uintptr_t ctrl  = MiscRd<uintptr_t>(chunk + 112 * (i & 0x1FF));
        if (!ctrl) continue;

        const uint32_t handle = MiscRd<uint32_t>(ctrl + CtrlOffsetsMisc::m_hPlayerPawn);
        if (!handle || handle == 0xFFFFFFFF) continue;

        const int pawnIdx = handle & 0x7FFF;
        const uintptr_t pChunk = MiscRd<uintptr_t>(entList + 16 + 8 * ((pawnIdx & 0x7FFF) >> 9));
        if (!pChunk) continue;

        const uintptr_t pPawn = MiscRd<uintptr_t>(pChunk + 112 * (pawnIdx & 0x1FF));
        if (!pPawn || pPawn == localPawn) continue;
        if (MiscRd<uint8_t>(pPawn + PawnOffsetsMisc::m_lifeState) != 0) continue;

        const uintptr_t node = MiscRd<uintptr_t>(pPawn + PawnOffsetsMisc::m_pGameSceneNode);
        if (!node) continue;

        Vec3 pos = MiscRd<Vec3>(node + NodeOffsetsMisc::m_vRenderOrigin);
        Vec3 diff = pos - localPos;

        float radarX = radarCenter.x + (diff.x * mapScale);
        float radarY = radarCenter.y + (diff.y * mapScale);

        // Clamp to radar bounds
        if (radarX < radarCenter.x - 80.f) radarX = radarCenter.x - 80.f;
        if (radarX > radarCenter.x + 80.f) radarX = radarCenter.x + 80.f;
        if (radarY < radarCenter.y - 80.f) radarY = radarCenter.y - 80.f;
        if (radarY > radarCenter.y + 80.f) radarY = radarCenter.y + 80.f;

        int team = MiscRd<int>(pPawn + PawnOffsetsMisc::m_iTeamNum);
        ImU32 col = (team == 2) ? IM_COL32(255, 75, 75, 200) : IM_COL32(75, 160, 255, 200);

        dl->AddCircleFilled({ radarX, radarY }, 2.5f, col, 6);
        radarCount++;
    }
}

void Misc::Update()
{
    // Bomb timer, spectator list, and other misc features are stubs for now
    // Can be implemented when needed
}

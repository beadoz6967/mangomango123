// features/aimbot.cpp — basic head aimbot with FOV check + smoothing
#include "../pch.h"
#include "aimbot.hpp"
#include "../gui/gui.h"
#include "../sdk/offsets.hpp"
#include <Windows.h>
#include <cmath>

using namespace cs2_dumper::offsets;

struct Vec3 { float x, y, z; Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; } };

template<typename T>
static T AimRd(uintptr_t addr) {
    T r{};
    if (addr < 0x100000 || addr > 0x7FFFFFFEFFFF) return r;
    __try { r = *reinterpret_cast<T*>(addr); } __except(EXCEPTION_EXECUTE_HANDLER) { return T{}; }
    return r;
}

namespace PawnOffA {
    constexpr ptrdiff_t m_iHealth        = 0x334;
    constexpr ptrdiff_t m_iTeamNum       = 0x3CB;
    constexpr ptrdiff_t m_lifeState      = 0x338;
    constexpr ptrdiff_t m_pGameSceneNode = 0x310;
    constexpr ptrdiff_t m_vRenderOrigin  = 0x128;
}
namespace CtrlOffA { constexpr ptrdiff_t m_hPlayerPawn = 0x7E4; }

static float NormAngle(float a) {
    while (a >  180.f) a -= 360.f;
    while (a < -180.f) a += 360.f;
    return a;
}

static Vec3 CalcAngle(Vec3 src, Vec3 dst) {
    Vec3 d = dst - src;
    float pitch = -atan2f(d.z, sqrtf(d.x*d.x + d.y*d.y)) * 180.f / 3.14159265f;
    float yaw   =  atan2f(d.y, d.x)                       * 180.f / 3.14159265f;
    return { pitch, yaw, 0.f };
}

namespace Aimbot {
    void Update() {
        if (!GUI::bAimbot) return;
        if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000)) return;

        const auto clientBase = reinterpret_cast<uintptr_t>(GetModuleHandleA("client.dll"));
        if (!clientBase) return;

        const uintptr_t localPawn = AimRd<uintptr_t>(clientBase + client_dll::dwLocalPlayerPawn);
        if (!localPawn) return;

        const uintptr_t entList = AimRd<uintptr_t>(clientBase + client_dll::dwEntityList);
        if (!entList) return;

        const uintptr_t localNode = AimRd<uintptr_t>(localPawn + PawnOffA::m_pGameSceneNode);
        if (!localNode) return;
        Vec3 localFeet = AimRd<Vec3>(localNode + PawnOffA::m_vRenderOrigin);
        Vec3 localEyes = { localFeet.x, localFeet.y, localFeet.z + 64.f };

        auto* pAngles       = reinterpret_cast<Vec3*>(clientBase + client_dll::dwViewAngles);
        Vec3  currentAngles = AimRd<Vec3>(reinterpret_cast<uintptr_t>(pAngles));

        int localTeam = AimRd<int>(localPawn + PawnOffA::m_iTeamNum);

        uintptr_t bestTarget = 0;
        float     bestFov    = GUI::fAimFOV;
        Vec3      bestAngle  = {};

        for (int i = 1; i < 128; i++) {
            const uintptr_t chunk = AimRd<uintptr_t>(entList + 16 + 8 * ((i & 0x7FFF) >> 9));
            if (!chunk) continue;
            const uintptr_t ctrl  = AimRd<uintptr_t>(chunk + 112 * (i & 0x1FF));
            if (!ctrl) continue;

            const uint32_t  handle  = AimRd<uint32_t>(ctrl + CtrlOffA::m_hPlayerPawn);
            if (!handle || handle == 0xFFFFFFFF) continue;

            const int       pawnIdx = handle & 0x7FFF;
            const uintptr_t pChunk  = AimRd<uintptr_t>(entList + 16 + 8 * ((pawnIdx & 0x7FFF) >> 9));
            if (!pChunk) continue;

            const uintptr_t pPawn = AimRd<uintptr_t>(pChunk + 112 * (pawnIdx & 0x1FF));
            if (!pPawn || pPawn == localPawn) continue;
            if (AimRd<uint8_t>(pPawn + PawnOffA::m_lifeState) != 0) continue;

            const int hp = AimRd<int>(pPawn + PawnOffA::m_iHealth);
            if (hp <= 0 || hp > 100) continue;

            if (GUI::bAimbotTeamCheck && AimRd<int>(pPawn + PawnOffA::m_iTeamNum) == localTeam)
                continue;

            const uintptr_t node = AimRd<uintptr_t>(pPawn + PawnOffA::m_pGameSceneNode);
            if (!node) continue;

            Vec3 feet   = AimRd<Vec3>(node + PawnOffA::m_vRenderOrigin);
            Vec3 target = feet;
            target.z   += (GUI::iAimbotBone == 1) ? 72.f : 48.f;

            Vec3  targetAngle = CalcAngle(localEyes, target);
            Vec3  delta       = { NormAngle(targetAngle.x - currentAngles.x),
                                  NormAngle(targetAngle.y - currentAngles.y), 0.f };
            float fovDist     = sqrtf(delta.x*delta.x + delta.y*delta.y);

            if (fovDist < bestFov) {
                bestFov    = fovDist;
                bestTarget = pPawn;
                bestAngle  = targetAngle;
            }
        }

        if (!bestTarget) return;

        Vec3 newAngles = currentAngles;
        newAngles.x += NormAngle(bestAngle.x - currentAngles.x) / GUI::fAimSmooth;
        newAngles.y += NormAngle(bestAngle.y - currentAngles.y) / GUI::fAimSmooth;

        if (newAngles.x >  89.f) newAngles.x =  89.f;
        if (newAngles.x < -89.f) newAngles.x = -89.f;

        *pAngles = newAngles;
    }
}

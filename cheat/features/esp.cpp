#include "../pch.h"
#include "esp.hpp"
#include "../gui/gui.h"
#include "../sdk/offsets.hpp"
#include "../imgui/imgui.h"
#include <Windows.h>

using namespace cs2_dumper::offsets;

struct Vector3 {
    float x, y, z;
    Vector3 operator+(const Vector3& o) const { return { x + o.x, y + o.y, z + o.z }; }
};

struct VMatrix { float m[4][4]; };

namespace PawnOff {
    constexpr ptrdiff_t m_iHealth        = 0x334;
    constexpr ptrdiff_t m_iTeamNum       = 0x3CB;
    constexpr ptrdiff_t m_lifeState      = 0x338;
    constexpr ptrdiff_t m_pGameSceneNode = 0x310;
}

namespace CtrlOff {
    constexpr ptrdiff_t m_hPlayerPawn = 0x7E4; // CCSPlayerController -> m_hPlayerPawn
}

namespace NodeOff {
    constexpr ptrdiff_t m_bDormant      = 0x103;
    constexpr ptrdiff_t m_vRenderOrigin = 0x128;
}

template<typename T>
static __forceinline T Rd(uintptr_t addr) { return *reinterpret_cast<T*>(addr); }

static bool W2S(const Vector3& world, ImVec2& screen, const VMatrix& vm, float sw, float sh) {
    float x = vm.m[0][0]*world.x + vm.m[0][1]*world.y + vm.m[0][2]*world.z + vm.m[0][3];
    float y = vm.m[1][0]*world.x + vm.m[1][1]*world.y + vm.m[1][2]*world.z + vm.m[1][3];
    float w = vm.m[3][0]*world.x + vm.m[3][1]*world.y + vm.m[3][2]*world.z + vm.m[3][3];

    if (w < 0.001f) return false;

    const float inv = 1.f / w;
    screen.x = (sw * 0.5f) + x * inv * (sw * 0.5f);
    screen.y = (sh * 0.5f) - y * inv * (sh * 0.5f);
    return true;
}

void ESP::Render() {
    if (!GUI::bBoxESP && !GUI::bHealthBar) return;

    const auto clientBase  = reinterpret_cast<uintptr_t>(GetModuleHandleA("client.dll"));
    const auto engine2Base = reinterpret_cast<uintptr_t>(GetModuleHandleA("engine2.dll"));
    if (!clientBase || !engine2Base) return;

    float sw = static_cast<float>(Rd<int>(engine2Base + engine2_dll::dwWindowWidth));
    float sh = static_cast<float>(Rd<int>(engine2Base + engine2_dll::dwWindowHeight));
    if (sw < 640.f || sh < 480.f) {
        sw = ImGui::GetIO().DisplaySize.x;
        sh = ImGui::GetIO().DisplaySize.y;
    }

    const auto vm        = Rd<VMatrix>(clientBase + client_dll::dwViewMatrix);
    const auto localPawn = Rd<uintptr_t>(clientBase + client_dll::dwLocalPlayerPawn);
    const auto entSys    = Rd<uintptr_t>(clientBase + client_dll::dwGameEntitySystem);
    if (!entSys) return;

    const int rawIdx     = Rd<int>(entSys + client_dll::dwGameEntitySystem_highestEntityIndex);
    const int highestIdx = rawIdx < 64 ? rawIdx : 64;

    ImGui::SetNextWindowPos({ 0.f, 0.f });
    ImGui::SetNextWindowSize({ sw, sh });
    ImGui::SetNextWindowBgAlpha(0.f);
    ImGui::Begin("##esp_ov", nullptr,
        ImGuiWindowFlags_NoTitleBar             |
        ImGuiWindowFlags_NoInputs               |
        ImGuiWindowFlags_NoScrollbar            |
        ImGuiWindowFlags_NoSavedSettings        |
        ImGuiWindowFlags_NoDecoration           |
        ImGuiWindowFlags_NoBringToFrontOnFocus  |
        ImGuiWindowFlags_NoNav
    );
    ImDrawList* dl = ImGui::GetWindowDrawList();

    for (int i = 1; i <= highestIdx; i++) {
        __try {
            const int chunk = i >> 9;
            const int slot  = i & 0x1FF;

            // controller lives at chunkPtr + slot * 0x78
            const uintptr_t chunkPtr = Rd<uintptr_t>(entSys + 0x10 + chunk * 8);
            if (!chunkPtr) continue;

            const uintptr_t controller = Rd<uintptr_t>(chunkPtr + slot * 0x78);
            if (!controller) continue;

            // resolve controller → pawn via m_hPlayerPawn handle
            const uint32_t pawnHandle = Rd<uint32_t>(controller + CtrlOff::m_hPlayerPawn);
            if ((pawnHandle & 0xFFFFFF) == 0xFFFFFF) continue; // invalid handle sentinel

            const int pawnIdx        = pawnHandle & 0x7FFF;
            const uintptr_t pawnChunk = Rd<uintptr_t>(entSys + 0x10 + (pawnIdx >> 9) * 8);
            if (!pawnChunk) continue;

            const uintptr_t pPawn = Rd<uintptr_t>(pawnChunk + (pawnIdx & 0x1FF) * 0x78);
            if (!pPawn || pPawn == localPawn) continue;

            if (Rd<uint8_t>(pPawn + PawnOff::m_lifeState) != 0) continue;

            const int hp = Rd<int>(pPawn + PawnOff::m_iHealth);
            if (hp <= 0 || hp > 100) continue;

            const int team = Rd<int>(pPawn + PawnOff::m_iTeamNum);

            const uintptr_t sceneNode = Rd<uintptr_t>(pPawn + PawnOff::m_pGameSceneNode);
            if (!sceneNode) continue;
            if (Rd<uint8_t>(sceneNode + NodeOff::m_bDormant)) continue;

            const Vector3 feet = Rd<Vector3>(sceneNode + NodeOff::m_vRenderOrigin);
            const Vector3 head = feet + Vector3{ 0.f, 0.f, 72.f };

            ImVec2 sFeet, sHead;
            if (!W2S(feet, sFeet, vm, sw, sh)) continue;
            if (!W2S(head, sHead, vm, sw, sh)) continue;
            if (sHead.y >= sFeet.y)            continue;

            const float boxH = sFeet.y - sHead.y;
            const float boxW = boxH * 0.40f;
            const float boxX = sHead.x - boxW * 0.5f;
            const float boxY = sHead.y;

            ImU32 col;
            switch (team) {
                case 2:  col = IM_COL32(255, 75,  75,  230); break;
                case 3:  col = IM_COL32(75,  160, 255, 230); break;
                default: col = IM_COL32(220, 220, 220, 200); break;
            }

            if (GUI::bBoxESP) {
                dl->AddRect(
                    { boxX - 1.f, boxY - 1.f },
                    { boxX + boxW + 1.f, boxY + boxH + 1.f },
                    IM_COL32(0, 0, 0, 150), 0.f, 0, 2.8f
                );
                dl->AddRect(
                    { boxX, boxY },
                    { boxX + boxW, boxY + boxH },
                    col, 0.f, 0, 1.6f
                );
            }

            if (GUI::bHealthBar) {
                constexpr float bw   = 4.f;
                constexpr float gap  = 5.f;
                const float     barX = boxX - gap - bw;
                const float     fillH = boxH * (hp / 100.f);

                dl->AddRectFilled(
                    { barX,      boxY },
                    { barX + bw, boxY + boxH },
                    IM_COL32(0, 0, 0, 170)
                );
                const float t = hp / 100.f;
                const auto  r = static_cast<uint8_t>((1.f - t) * 255.f);
                const auto  g = static_cast<uint8_t>(t * 210.f);
                dl->AddRectFilled(
                    { barX,      boxY + boxH - fillH },
                    { barX + bw, boxY + boxH         },
                    IM_COL32(r, g, 30, 225)
                );
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) { continue; }
    }

    ImGui::End();
}
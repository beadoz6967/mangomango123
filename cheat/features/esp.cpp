// features/esp.cpp — fixed W2S, fixed distance, fixed offscreen arrows
#include "../pch.h"
#include "esp.hpp"
#include "../gui/gui.h"
#include "../sdk/offsets.hpp"
#include "../imgui/imgui.h"
#include "wallhack.hpp"
#include "glow.hpp"
#include <Windows.h>
#include <cmath>
#include <string>

using namespace cs2_dumper::offsets;

struct Vector3 {
    float x, y, z;
    Vector3 operator+(const Vector3& o) const { return { x+o.x, y+o.y, z+o.z }; }
    Vector3 operator-(const Vector3& o) const { return { x-o.x, y-o.y, z-o.z }; }
    Vector3 operator*(float s)          const { return { x*s, y*s, z*s }; }
    float Length() const { return std::sqrt(x*x + y*y + z*z); }
};

struct VMatrix { float m[4][4]; };

namespace PawnOff {
    constexpr ptrdiff_t m_iHealth        = 0x334;
    constexpr ptrdiff_t m_iTeamNum       = 0x3CB;
    constexpr ptrdiff_t m_lifeState      = 0x338;
    constexpr ptrdiff_t m_pGameSceneNode = 0x310;
    constexpr ptrdiff_t m_iszPlayerName  = 0x600;
    constexpr ptrdiff_t m_pClippingWeapon= 0x9C8;
}
namespace CtrlOff  { constexpr ptrdiff_t m_hPlayerPawn = 0x7E4; }
namespace NodeOff  { constexpr ptrdiff_t m_bDormant = 0x103; constexpr ptrdiff_t m_vRenderOrigin = 0x128; }
namespace WeaponOff{ constexpr ptrdiff_t m_iClipAmmo = 0x330; constexpr ptrdiff_t m_iPrimaryReserveAmmoCount = 0x33C; }

template<typename T>
static T Rd(uintptr_t addr) {
    T result{};
    if (addr < 0x100000 || addr > 0x7FFFFFFEFFFF) return result;
    __try   { result = *reinterpret_cast<T*>(addr); }
    __except(EXCEPTION_EXECUTE_HANDLER) { return T{}; }
    return result;
}

// FIXED: correct W2S — no double sw/2 offset
static bool W2S(const Vector3& world, ImVec2& screen, const VMatrix& vm, float sw, float sh) {
    float w = vm.m[3][0]*world.x + vm.m[3][1]*world.y + vm.m[3][2]*world.z + vm.m[3][3];
    if (w < 0.01f) return false;
    float x = vm.m[0][0]*world.x + vm.m[0][1]*world.y + vm.m[0][2]*world.z + vm.m[0][3];
    float y = vm.m[1][0]*world.x + vm.m[1][1]*world.y + vm.m[1][2]*world.z + vm.m[1][3];
    float inv = 1.0f / w;
    screen.x = (sw * 0.5f) + (x * inv * sw * 0.5f);
    screen.y = (sh * 0.5f) - (y * inv * sh * 0.5f);
    return true;
}

static bool SafeCopyName(const char* src, char* dst, int max) {
    __try {
        for (int i = 0; i < max && src[i]; i++) dst[i] = src[i];
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static std::string GetPlayerName(uintptr_t pPawn) {
    const char* ptr = Rd<const char*>(pPawn + PawnOff::m_iszPlayerName);
    if (!ptr) return "Unknown";
    char buf[128]{};
    if (!SafeCopyName(ptr, buf, 127) || !buf[0]) return "Unknown";
    return buf;
}

static void GetAmmoInfo(uintptr_t weapon, int& clip, int& reserve) {
    if (!weapon) return;
    clip    = Rd<int>(weapon + WeaponOff::m_iClipAmmo);
    reserve = Rd<int>(weapon + WeaponOff::m_iPrimaryReserveAmmoCount);
}

void ESP::Render() {
    if (!GUI::bBoxESP && !GUI::bHealthBar && !GUI::bGlowESP && !GUI::bWallhackESP &&
        !GUI::bPlayerNames && !GUI::bWeaponInfo && !GUI::bDistance && !GUI::bArrows)
        return;

    float sw = ImGui::GetIO().DisplaySize.x;
    float sh = ImGui::GetIO().DisplaySize.y;
    if (sw < 100.f || sh < 100.f) return;

    ImGui::SetNextWindowPos({ 0.f, 0.f });
    ImGui::SetNextWindowSize({ sw, sh });
    ImGui::Begin("##esp_ov", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    const auto clientBase = reinterpret_cast<uintptr_t>(GetModuleHandleA("client.dll"));
    if (!clientBase) { ImGui::End(); return; }

    const uintptr_t entList   = Rd<uintptr_t>(clientBase + client_dll::dwEntityList);
    const uintptr_t localPawn = Rd<uintptr_t>(clientBase + client_dll::dwLocalPlayerPawn);
    const auto      vm        = Rd<VMatrix>  (clientBase + client_dll::dwViewMatrix);

    if (!entList || !localPawn) { ImGui::End(); return; }

    // FIXED: follow scene node pointer to get local position
    const uintptr_t localNode = Rd<uintptr_t>(localPawn + PawnOff::m_pGameSceneNode);
    const Vector3   localPos  = localNode ? Rd<Vector3>(localNode + NodeOff::m_vRenderOrigin) : Vector3{};

    int pawnsDrawn = 0;

    for (int i = 1; i < 128; i++) {
        const uintptr_t chunkPtr   = Rd<uintptr_t>(entList + 16 + 8 * ((i & 0x7FFF) >> 9));
        if (!chunkPtr) continue;
        const uintptr_t controller = Rd<uintptr_t>(chunkPtr + 112 * (i & 0x1FF));
        if (!controller) continue;

        const uint32_t  pawnHandle = Rd<uint32_t>(controller + CtrlOff::m_hPlayerPawn);
        if (!pawnHandle || pawnHandle == 0xFFFFFFFF) continue;

        const int       pawnIdx    = pawnHandle & 0x7FFF;
        const uintptr_t pawnChunk  = Rd<uintptr_t>(entList + 16 + 8 * ((pawnIdx & 0x7FFF) >> 9));
        if (!pawnChunk) continue;

        const uintptr_t pPawn = Rd<uintptr_t>(pawnChunk + 112 * (pawnIdx & 0x1FF));
        if (!pPawn || pPawn == localPawn) continue;
        if (Rd<uint8_t>(pPawn + PawnOff::m_lifeState) != 0) continue;

        int hp = Rd<int>(pPawn + PawnOff::m_iHealth);
        if (hp <= 0 || hp > 100) continue;

        const uintptr_t sceneNode = Rd<uintptr_t>(pPawn + PawnOff::m_pGameSceneNode);
        if (!sceneNode) continue;

        Vector3 feet = Rd<Vector3>(sceneNode + NodeOff::m_vRenderOrigin);
        Vector3 head = feet + Vector3{ 0.f, 0.f, 72.f };

        ImVec2 sFeet, sHead;
        if (!W2S(feet, sFeet, vm, sw, sh) || !W2S(head, sHead, vm, sw, sh)) continue;

        float boxH = std::abs(sFeet.y - sHead.y);
        if (boxH < 2.0f) continue;

        float boxW = boxH * 0.45f;
        float boxX = sHead.x - (boxW * 0.5f);

        int   team = Rd<int>(pPawn + PawnOff::m_iTeamNum);
        ImU32 col  = (team == 2) ? IM_COL32(255, 75, 75, 230) : IM_COL32(75, 160, 255, 230);

        bool  dormant    = Rd<uint8_t>(sceneNode + NodeOff::m_bDormant) != 0;
        float visibility = dormant ? 0.3f : 1.0f;

        if (GUI::bGlowESP)    Glow::RenderGlow      (dl, boxX, sHead.y, boxW, boxH, col, 0.9f);
        if (GUI::bWallhackESP)Wallhack::RenderWallhack(dl, boxX, sHead.y, boxW, boxH, col, visibility, 0.9f);

        if (GUI::bBoxESP) {
            dl->AddRect({ boxX,   sHead.y   }, { boxX + boxW,     sFeet.y     }, col,            0.f, 0, 1.2f);
            dl->AddRect({ boxX-1, sHead.y-1 }, { boxX + boxW + 1, sFeet.y + 1 }, IM_COL32(0,0,0,150));
        }

        if (GUI::bHealthBar) {
            float fill = (hp / 100.f) * boxH;
            dl->AddRectFilled({ boxX-6, sHead.y        }, { boxX-2, sFeet.y        }, IM_COL32(0,0,0,180));
            dl->AddRectFilled({ boxX-6, sFeet.y - fill }, { boxX-2, sFeet.y        }, IM_COL32(0,255,0,255));
        }

        if (GUI::bPlayerNames) {
            std::string name = GetPlayerName(pPawn);
            ImVec2 ts = ImGui::CalcTextSize(name.c_str());
            dl->AddText({ boxX + boxW * 0.5f - ts.x * 0.5f, sHead.y - 20.f }, col, name.c_str());
        }

        if (GUI::bWeaponInfo) {
            uintptr_t weapon = Rd<uintptr_t>(pPawn + PawnOff::m_pClippingWeapon);
            if (weapon) {
                int clip = 0, reserve = 0;
                GetAmmoInfo(weapon, clip, reserve);
                std::string ammo = std::to_string(clip) + "/" + std::to_string(reserve);
                ImVec2 ts = ImGui::CalcTextSize(ammo.c_str());
                dl->AddText({ boxX + boxW * 0.5f - ts.x * 0.5f, sFeet.y + 5.f }, col, ammo.c_str());
            }
        }

        if (GUI::bDistance) {
            // FIXED: was reading m_pGameSceneNode offset as Vector3 directly — now follows pointer
            float dist = (feet - localPos).Length() / 39.37f;
            std::string ds = std::to_string((int)dist) + "m";
            dl->AddText({ boxX + boxW + 5.f, sHead.y }, col, ds.c_str());
        }

        // FIXED: only draw offscreen arrow when player is actually off-screen
        if (GUI::bArrows) {
            bool onScreen = (sHead.x >= 0 && sHead.x <= sw && sHead.y >= 0 && sHead.y <= sh);
            if (!onScreen) {
                ImVec2 center = { sw * 0.5f, sh * 0.5f };
                ImVec2 dir    = { sHead.x - center.x, sHead.y - center.y };
                float  len    = std::sqrt(dir.x*dir.x + dir.y*dir.y);
                if (len > 0.f) {
                    dir.x /= len; dir.y /= len;
                    ImVec2 ap   = { center.x + dir.x * 150.f, center.y + dir.y * 150.f };
                    float  sz   = 10.f;
                    ImVec2 perp = { -dir.y * sz, dir.x * sz };
                    dl->AddTriangleFilled(
                        { ap.x + dir.x * sz, ap.y + dir.y * sz },
                        { ap.x - perp.x,     ap.y - perp.y     },
                        { ap.x + perp.x,     ap.y + perp.y     },
                        col
                    );
                }
            }
        }

        pawnsDrawn++;
    }

    ImGui::SetCursorPos({ 20.f, 20.f });
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "Pawns Rendered: %d", pawnsDrawn);
    ImGui::End();
}

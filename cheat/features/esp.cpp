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
    Vector3 operator+(const Vector3& o) const { return { x + o.x, y + o.y, z + o.z }; }
    Vector3 operator-(const Vector3& o) const { return { x - o.x, y - o.y, z - o.z }; }
    Vector3 operator*(float s) const { return { x * s, y * s, z * s }; }
    float Length() const { return std::sqrt(x * x + y * y + z * z); }
    Vector3 Normalized() const { float len = Length(); return len > 0 ? Vector3{ x / len, y / len, z / len } : Vector3{ 0, 0, 0 }; }
};

struct VMatrix { float m[4][4]; };

namespace PawnOff {
    constexpr ptrdiff_t m_iHealth = 0x334;
    constexpr ptrdiff_t m_iTeamNum = 0x3CB;
    constexpr ptrdiff_t m_lifeState = 0x338;
    constexpr ptrdiff_t m_pGameSceneNode = 0x310;
    constexpr ptrdiff_t m_iszPlayerName = 0x600;
    constexpr ptrdiff_t m_pClippingWeapon = 0x9C8;
}

namespace CtrlOff {
    constexpr ptrdiff_t m_hPlayerPawn = 0x7E4;
}

namespace NodeOff {
    constexpr ptrdiff_t m_bDormant = 0x103;
    constexpr ptrdiff_t m_vRenderOrigin = 0x128;
}

namespace WeaponOff {
    constexpr ptrdiff_t m_iClipAmmo = 0x330;
    constexpr ptrdiff_t m_iPrimaryReserveAmmoCount = 0x33C;
}

// Safe memory read - prevents crashes
template<typename T>
static T Rd(uintptr_t addr) {
    T result{};
    if (addr < 0x100000 || addr > 0x7FFFFFFEFFFF) return result;

    __try {
        result = *reinterpret_cast<T*>(addr);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return T{};
    }
    return result;
}

// World to Screen projection
static bool W2S(const Vector3& world, ImVec2& screen, const VMatrix& vm, float sw, float sh) {
    float w = vm.m[3][0] * world.x + vm.m[3][1] * world.y + vm.m[3][2] * world.z + vm.m[3][3];
    if (w < 0.01f) return false;

    float x = vm.m[0][0] * world.x + vm.m[0][1] * world.y + vm.m[0][2] * world.z + vm.m[0][3];
    float y = vm.m[1][0] * world.x + vm.m[1][1] * world.y + vm.m[1][2] * world.z + vm.m[1][3];

    float invw = 1.0f / w;
    screen.x = (sw * 0.5f) + (0.5f * x * invw * sw + 0.5f);
    screen.y = (sh * 0.5f) - (0.5f * y * invw * sh + 0.5f);
    return true;
}

// Get player name safely
static std::string GetPlayerName(uintptr_t pPawn) {
    const char* name = Rd<const char*>(pPawn + PawnOff::m_iszPlayerName);
    if (!name || name[0] == '\0') return "Unknown";
    return std::string(name);
}

// Get weapon name safely
static std::string GetWeaponName(uintptr_t weapon) {
    if (!weapon) return "Unarmed";
    // Simplified - in real scenario you'd read from entity class name
    return "Weapon";
}

// Get ammo info
static void GetAmmoInfo(uintptr_t weapon, int& clip, int& reserve) {
    if (!weapon) return;
    clip = Rd<int>(weapon + WeaponOff::m_iClipAmmo);
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

    const uintptr_t entList = Rd<uintptr_t>(clientBase + client_dll::dwEntityList);
    const uintptr_t localPawn = Rd<uintptr_t>(clientBase + client_dll::dwLocalPlayerPawn);
    const auto vm = Rd<VMatrix>(clientBase + client_dll::dwViewMatrix);

    if (!entList || !localPawn) {
        ImGui::End();
        return;
    }

    int pawnsDrawn = 0;

    // Loop through the controller slots
    for (int i = 1; i < 128; i++) {
        // Calculate chunk pointer with proper masking
        const uintptr_t chunkPtr = Rd<uintptr_t>(entList + 16 + 8 * ((i & 0x7FFF) >> 9));
        if (!chunkPtr) continue;

        // Get controller from chunk with 112-byte stride
        const uintptr_t controller = Rd<uintptr_t>(chunkPtr + 112 * (i & 0x1FF));
        if (!controller) continue;

        // Get pawn handle from controller
        const uint32_t pawnHandle = Rd<uint32_t>(controller + CtrlOff::m_hPlayerPawn);
        if (pawnHandle == 0 || pawnHandle == 0xFFFFFFFF) continue;

        // Extract pawn index from handle
        const int pawnIdx = pawnHandle & 0x7FFF;
        const uintptr_t pawnChunk = Rd<uintptr_t>(entList + 16 + 8 * ((pawnIdx & 0x7FFF) >> 9));
        if (!pawnChunk) continue;

        // Get pawn from chunk
        const uintptr_t pPawn = Rd<uintptr_t>(pawnChunk + 112 * (pawnIdx & 0x1FF));
        if (!pPawn || pPawn == localPawn) continue;

        // Check if alive
        if (Rd<uint8_t>(pPawn + PawnOff::m_lifeState) != 0) continue;

        int hp = Rd<int>(pPawn + PawnOff::m_iHealth);
        if (hp <= 0 || hp > 100) continue;

        // Get position
        const uintptr_t sceneNode = Rd<uintptr_t>(pPawn + PawnOff::m_pGameSceneNode);
        if (!sceneNode) continue;

        Vector3 feet = Rd<Vector3>(sceneNode + NodeOff::m_vRenderOrigin);
        Vector3 head = feet + Vector3{ 0.f, 0.f, 72.f };

        // Project to screen
        ImVec2 sFeet, sHead;
        if (!W2S(feet, sFeet, vm, sw, sh) || !W2S(head, sHead, vm, sw, sh))
            continue;

        float boxH = std::abs(sFeet.y - sHead.y);
        if (boxH < 2.0f) continue;

        float boxW = boxH * 0.45f;
        float boxX = sHead.x - (boxW / 2.0f);

        // Get team color
        int team = Rd<int>(pPawn + PawnOff::m_iTeamNum);
        ImU32 col = (team == 2) ? IM_COL32(255, 75, 75, 230) : IM_COL32(75, 160, 255, 230);

        // Check dormancy
        float visibility = 1.0f;
        if (Rd<uint8_t>(pPawn + NodeOff::m_bDormant)) {
            visibility = 0.3f;
        }

        float baseAlpha = 0.9f;

        // Draw glow
        if (GUI::bGlowESP) {
            Glow::RenderGlow(dl, boxX, sHead.y, boxW, boxH, col, baseAlpha);
        }

        // Draw wallhack
        if (GUI::bWallhackESP) {
            Wallhack::RenderWallhack(dl, boxX, sHead.y, boxW, boxH, col, visibility, baseAlpha);
        }

        // Draw box
        if (GUI::bBoxESP) {
            dl->AddRect({ boxX, sHead.y }, { boxX + boxW, sFeet.y }, col, 0.0f, 0, 1.2f);
            dl->AddRect({ boxX - 1, sHead.y - 1 }, { boxX + boxW + 1, sFeet.y + 1 }, IM_COL32(0, 0, 0, 150));
        }

        // Draw health bar
        if (GUI::bHealthBar) {
            float fill = (static_cast<float>(hp) / 100.0f) * boxH;
            dl->AddRectFilled({ boxX - 6, sHead.y }, { boxX - 2, sFeet.y }, IM_COL32(0, 0, 0, 180));
            dl->AddRectFilled({ boxX - 6, sFeet.y - fill }, { boxX - 2, sFeet.y }, IM_COL32(0, 255, 0, 255));
        }

        // Draw player name
        if (GUI::bPlayerNames) {
            std::string name = GetPlayerName(pPawn);
            ImVec2 textSize = ImGui::CalcTextSize(name.c_str());
            dl->AddText({ boxX + boxW / 2 - textSize.x / 2, sHead.y - 20 }, col, name.c_str());
        }

        // Draw weapon info
        if (GUI::bWeaponInfo) {
            uintptr_t weapon = Rd<uintptr_t>(pPawn + PawnOff::m_pClippingWeapon);
            if (weapon) {
                int clip = 0, reserve = 0;
                GetAmmoInfo(weapon, clip, reserve);
                std::string ammoStr = std::to_string(clip) + "/" + std::to_string(reserve);
                ImVec2 textSize = ImGui::CalcTextSize(ammoStr.c_str());
                dl->AddText({ boxX + boxW / 2 - textSize.x / 2, sFeet.y + 5 }, col, ammoStr.c_str());
            }
        }

        // Draw distance
        if (GUI::bDistance) {
            Vector3 dist = feet - Rd<Vector3>(localPawn + PawnOff::m_pGameSceneNode);
            float distance = dist.Length();
            std::string distStr = std::to_string((int)(distance / 39.37f)) + "m"; // Convert to meters
            ImVec2 textSize = ImGui::CalcTextSize(distStr.c_str());
            dl->AddText({ boxX + boxW + 5, sHead.y }, col, distStr.c_str());
        }

        // Draw offscreen arrow
        if (GUI::bArrows) {
            ImVec2 screenCenter = { sw / 2, sh / 2 };
            ImVec2 direction = { sHead.x - screenCenter.x, sHead.y - screenCenter.y };
            float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (len > 0) {
                direction.x /= len;
                direction.y /= len;

                ImVec2 arrowPos = { screenCenter.x + direction.x * 150, screenCenter.y + direction.y * 150 };
                float arrowSize = 10.0f;
                dl->AddTriangleFilled(
                    { arrowPos.x, arrowPos.y - arrowSize },
                    { arrowPos.x - arrowSize, arrowPos.y + arrowSize },
                    { arrowPos.x + arrowSize, arrowPos.y + arrowSize },
                    col
                );
            }
        }

        pawnsDrawn++;
    }

    // Draw stats
    ImGui::SetCursorPos({ 20.f, 20.f });
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "Pawns Rendered: %d", pawnsDrawn);

    ImGui::End();
}
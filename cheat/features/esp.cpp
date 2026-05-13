// features/esp.cpp
#include "../pch.h"
#include "esp.hpp"
#include "../gui/gui.h"
#include "../sdk/offsets.hpp"
#include "../sdk/schemas.hpp"
#include "../sdk/math.hpp"
#include "../imgui/imgui.h"
#include "wallhack.hpp"
#include "glow.hpp"
#include <Windows.h>
#include <cmath>
#include <string>

using namespace cs2_dumper::offsets;
using namespace schemas;

struct VMatrix { float m[4][4]; };

template<typename T>
static T Rd(uintptr_t addr) {
    T result{};
    if (addr < 0x100000 || addr > 0x7FFFFFFEFFFF) return result;
    __try   { result = *reinterpret_cast<T*>(addr); }
    __except(EXCEPTION_EXECUTE_HANDLER) { return T{}; }
    return result;
}

static bool W2S(const Vec3& world, ImVec2& screen, const VMatrix& vm, float sw, float sh) {
    float w = vm.m[3][0]*world.x + vm.m[3][1]*world.y + vm.m[3][2]*world.z + vm.m[3][3];
    if (w < 0.01f) return false;
    float x = vm.m[0][0]*world.x + vm.m[0][1]*world.y + vm.m[0][2]*world.z + vm.m[0][3];
    float y = vm.m[1][0]*world.x + vm.m[1][1]*world.y + vm.m[1][2]*world.z + vm.m[1][3];
    float inv = 1.0f / w;
    screen.x = (sw * 0.5f) + (x * inv * sw * 0.5f);
    screen.y = (sh * 0.5f) - (y * inv * sh * 0.5f);
    return true;
}

// m_iszPlayerName at 0x640 is inline char[128] on CCSPlayerController — no pointer hop.
static std::string GetPlayerName(uintptr_t ctrl) {
    const char* src = reinterpret_cast<const char*>(ctrl + controller::m_iszPlayerName);
    char buf[128]{};
    __try {
        for (int i = 0; i < 127 && src[i]; i++) buf[i] = src[i];
    } __except(EXCEPTION_EXECUTE_HANDLER) { return "Unknown"; }
    return buf[0] ? std::string(buf) : "Unknown";
}

static void GetAmmoInfo(uintptr_t wpn, int& clip, int& reserve) {
    if (!wpn) return;
    clip    = Rd<int>(wpn + weapon::m_iClipAmmo);
    reserve = Rd<int>(wpn + weapon::m_iPrimaryReserveAmmoCount);
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

    const uintptr_t localNode = Rd<uintptr_t>(localPawn + pawn::m_pGameSceneNode);
    const Vec3      localPos  = localNode ? Rd<Vec3>(localNode + node::m_vRenderOrigin) : Vec3{};

    // One-shot weapon stale warning — fires once per session after 5 consecutive bad frames.
    static int  s_badWeaponFrames = 0;
    static bool s_weaponWarned    = false;
    bool        anyBadWeaponRead  = false;

    for (int i = 1; i < 128; i++) {
        const uintptr_t chunkPtr = Rd<uintptr_t>(entList + 16 + 8 * ((i & 0x7FFF) >> 9));
        if (!chunkPtr) continue;
        const uintptr_t ctrl = Rd<uintptr_t>(chunkPtr + 112 * (i & 0x1FF));
        if (!ctrl) continue;

        const uint32_t  pawnHandle = Rd<uint32_t>(ctrl + controller::m_hPlayerPawn);
        if (!pawnHandle || pawnHandle == 0xFFFFFFFF) continue;

        const int       pawnIdx   = pawnHandle & 0x7FFF;
        const uintptr_t pawnChunk = Rd<uintptr_t>(entList + 16 + 8 * ((pawnIdx & 0x7FFF) >> 9));
        if (!pawnChunk) continue;

        const uintptr_t pPawn = Rd<uintptr_t>(pawnChunk + 112 * (pawnIdx & 0x1FF));
        if (!pPawn || pPawn == localPawn) continue;
        if (Rd<uint8_t>(pPawn + pawn::m_lifeState) != 0) continue;

        int hp = Rd<int>(pPawn + pawn::m_iHealth);
        if (hp <= 0 || hp > 100) continue;

        const uintptr_t sceneNode = Rd<uintptr_t>(pPawn + pawn::m_pGameSceneNode);
        if (!sceneNode) continue;

        Vec3 feet = Rd<Vec3>(sceneNode + node::m_vRenderOrigin);
        Vec3 head = feet + Vec3{ 0.f, 0.f, 72.f };

        ImVec2 sFeet, sHead;
        if (!W2S(feet, sFeet, vm, sw, sh) || !W2S(head, sHead, vm, sw, sh)) continue;

        float boxH = std::abs(sFeet.y - sHead.y);
        if (boxH < 2.0f) continue;

        float boxW = boxH * 0.45f;
        float boxX = sHead.x - (boxW * 0.5f);

        int   team = Rd<int>(pPawn + pawn::m_iTeamNum);
        ImU32 col  = (team == 2) ? IM_COL32(255, 75, 75, 230) : IM_COL32(75, 160, 255, 230);

        bool  dormant    = Rd<uint8_t>(sceneNode + node::m_bDormant) != 0;
        float visibility = dormant ? 0.3f : 1.0f;

        if (GUI::bGlowESP)     Glow::RenderGlow(dl, boxX, sHead.y, boxW, boxH, col, 0.9f);
        if (GUI::bWallhackESP) Wallhack::RenderWallhack(dl, boxX, sHead.y, boxW, boxH, col, visibility, 0.9f);

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
            std::string name = GetPlayerName(ctrl);
            ImVec2 ts = ImGui::CalcTextSize(name.c_str());
            dl->AddText({ boxX + boxW * 0.5f - ts.x * 0.5f, sHead.y - 20.f }, col, name.c_str());
        }

        if (GUI::bWeaponInfo) {
            uintptr_t wpn = Rd<uintptr_t>(pPawn + pawn::m_pClippingWeapon);
            if (wpn) {
                int clip = 0, reserve = 0;
                GetAmmoInfo(wpn, clip, reserve);

                if (!s_weaponWarned && clip == 0 && reserve == 0)
                    anyBadWeaponRead = true;

                std::string ammo = std::to_string(clip) + "/" + std::to_string(reserve);
                ImVec2 ts = ImGui::CalcTextSize(ammo.c_str());
                dl->AddText({ boxX + boxW * 0.5f - ts.x * 0.5f, sFeet.y + 5.f }, col, ammo.c_str());
            }
        }

        if (GUI::bDistance) {
            float dist = (feet - localPos).Length() / 39.37f;
            std::string ds = std::to_string((int)dist) + "m";
            dl->AddText({ boxX + boxW + 5.f, sHead.y }, col, ds.c_str());
        }

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
    }

    if (!s_weaponWarned && GUI::bWeaponInfo) {
        if (anyBadWeaponRead) s_badWeaponFrames++;
        else                  s_badWeaponFrames = 0;
        if (s_badWeaponFrames >= 5) {
            OutputDebugStringA("[mangomango123] m_pClippingWeapon may be stale");
            s_weaponWarned = true;
        }
    }

    ImGui::End();
}

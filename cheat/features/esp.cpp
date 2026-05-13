// features/esp.cpp — Osiris-style ESP, build 14160
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
    if (screen.x < -100.f || screen.x > sw + 100.f || screen.y < -100.f || screen.y > sh + 100.f)
        return false;
    return true;
}

static bool SafeCopyName(const char* src, char* dst, int max) {
    __try {
        for (int i = 0; i < max && src[i]; i++) dst[i] = src[i];
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static std::string GetPlayerName(uintptr_t ctrl) {
    char buf[128]{};
    const char* src = reinterpret_cast<const char*>(ctrl + controller::m_iszPlayerName);
    if (!SafeCopyName(src, buf, 127) || !buf[0]) return "Unknown";
    return std::string(buf);
}

static const char* GetWeaponName(uint16_t def) {
    switch (def) {
        case  1: return "Deagle";
        case  3: return "Five-Seven";
        case  4: return "Glock";
        case  7: return "AK-47";
        case  8: return "AUG";
        case  9: return "AWP";
        case 10: return "FAMAS";
        case 11: return "G3SG1";
        case 13: return "Galil";
        case 14: return "M249";
        case 16: return "M4A4";
        case 17: return "MAC-10";
        case 19: return "P90";
        case 23: return "MP5-SD";
        case 24: return "UMP";
        case 25: return "XM1014";
        case 26: return "PP-Bizon";
        case 27: return "MAG-7";
        case 28: return "Negev";
        case 30: return "Tec-9";
        case 32: return "P2000";
        case 33: return "MP7";
        case 34: return "MP9";
        case 35: return "Nova";
        case 36: return "P250";
        case 38: return "SCAR-20";
        case 39: return "SG553";
        case 40: return "SSG-08";
        case 43: return "Flash";
        case 44: return "HE";
        case 45: return "Smoke";
        case 46: return "Molotov";
        case 47: return "Decoy";
        case 48: return "Incendiary";
        case 49: return "C4";
        case 57: return "Healthshot";
        case 60: return "M4A1-S";
        case 61: return "USP-S";
        case 63: return "CZ75";
        case 64: return "R8";
        default: return nullptr;
    }
}

static uintptr_t ResolveActiveWeapon(uintptr_t pPawn, uintptr_t entList) {
    const uintptr_t wpnSvc = Rd<uintptr_t>(pPawn + pawn::m_pWeaponServices);
    if (!wpnSvc) return 0;
    const uint32_t handle = Rd<uint32_t>(wpnSvc + weapon_svc::m_hActiveWeapon);
    if (!handle || handle == 0xFFFFFFFF) return 0;
    const int       idx   = handle & 0x7FFF;
    const uintptr_t chunk = Rd<uintptr_t>(entList + 16 + 8 * ((idx & 0x7FFF) >> 9));
    if (!chunk) return 0;
    return Rd<uintptr_t>(chunk + 112 * (idx & 0x1FF));
}

static void DrawCornerBox(ImDrawList* dl, float x, float y, float w, float h,
                          ImU32 col, float thickness = 1.5f)
{
    float cw = w * 0.20f;
    float ch = h * 0.20f;
    dl->AddLine({x,   y  }, {x+cw, y  },    col, thickness);
    dl->AddLine({x,   y  }, {x,    y+ch},   col, thickness);
    dl->AddLine({x+w, y  }, {x+w-cw,y  },   col, thickness);
    dl->AddLine({x+w, y  }, {x+w,  y+ch},   col, thickness);
    dl->AddLine({x,   y+h}, {x+cw, y+h},    col, thickness);
    dl->AddLine({x,   y+h}, {x,    y+h-ch}, col, thickness);
    dl->AddLine({x+w, y+h}, {x+w-cw,y+h},   col, thickness);
    dl->AddLine({x+w, y+h}, {x+w,  y+h-ch}, col, thickness);
}

static void DrawHealthBar(ImDrawList* dl, float boxX, float top, float boxH,
                          int hp, bool showNum)
{
    float barX = boxX - 7.f;
    float bot  = top + boxH;
    float frac = std::max(0.f, std::min(1.f, hp / 100.f));
    float fillTop = bot - frac * boxH;

    dl->AddRectFilled({barX, top}, {barX + 4.f, bot}, IM_COL32(0,0,0,180));

    if (frac > 0.f) {
        float mid = top + boxH * 0.5f;  // y position at 50hp
        if (fillTop >= mid) {
            // only 0-50hp range visible: red to yellow
            float t = (bot - fillTop) / (boxH * 0.5f);
            int   g = (int)(255.f * t);
            dl->AddRectFilled({barX, fillTop}, {barX + 4.f, bot}, IM_COL32(255, g, 0, 230));
        } else {
            // 50-100hp: yellow to green on top, full red-to-yellow on bottom
            dl->AddRectFilled({barX, mid}, {barX + 4.f, bot}, IM_COL32(255, 255, 0, 230));
            float t = (mid - fillTop) / (boxH * 0.5f);
            int   r = (int)(255.f * (1.f - t));
            dl->AddRectFilled({barX, fillTop}, {barX + 4.f, mid}, IM_COL32(r, 255, 0, 230));
        }
    }

    if (showNum) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", hp);
        ImVec2 ts = ImGui::CalcTextSize(buf);
        dl->AddText({barX + 2.f - ts.x * 0.5f, bot + 2.f}, IM_COL32(255,255,255,200), buf);
    }
}

static void DrawArmorBar(ImDrawList* dl, float boxRight, float top, float boxH, int armor) {
    float barX = boxRight + 3.f;
    float bot  = top + boxH;
    float frac = std::max(0.f, std::min(1.f, armor / 100.f));

    dl->AddRectFilled({barX, top}, {barX + 4.f, bot}, IM_COL32(0,0,0,180));
    if (frac > 0.f)
        dl->AddRectFilled({barX, bot - frac * boxH}, {barX + 4.f, bot},
                          IM_COL32(100, 180, 255, 230));
}

void ESP::Render() {
    if (!GUI::bBoxESP     && !GUI::bHealthBar  && !GUI::bGlowESP    &&
        !GUI::bWallhackESP&& !GUI::bPlayerNames&& !GUI::bWeaponName &&
        !GUI::bWeaponInfo && !GUI::bDistance   && !GUI::bArrows     &&
        !GUI::bSnaplines  && !GUI::bArmorBar   && !GUI::bEspFlags)
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

    const ImVec2 snapBase = { sw * 0.5f, sh };

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
        if (Rd<uint8_t>(sceneNode + node::m_bDormant)) continue;

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

        // ── Effects ──────────────────────────────────────────────────────────
        if (GUI::bGlowESP)
            Glow::RenderGlow(dl, boxX, sHead.y, boxW, boxH, col, 0.9f);
        if (GUI::bWallhackESP)
            Wallhack::RenderWallhack(dl, boxX, sHead.y, boxW, boxH, col, 1.0f, 0.9f);

        // ── Snapline ─────────────────────────────────────────────────────────
        if (GUI::bSnaplines)
            dl->AddLine(snapBase, sFeet, IM_COL32(200,200,200,100), 1.0f);

        // ── Box ──────────────────────────────────────────────────────────────
        if (GUI::bBoxESP) {
            if (GUI::bCornerBox) {
                DrawCornerBox(dl, boxX-1.f, sHead.y-1.f, boxW+2.f, boxH+2.f,
                              IM_COL32(0,0,0,120), 2.8f);
                DrawCornerBox(dl, boxX, sHead.y, boxW, boxH, col, 1.5f);
            } else {
                dl->AddRect({boxX-1, sHead.y-1}, {boxX+boxW+1, sFeet.y+1}, IM_COL32(0,0,0,150));
                dl->AddRect({boxX,   sHead.y  }, {boxX+boxW,   sFeet.y  }, col, 0.f, 0, 1.2f);
            }
        }

        // ── Health bar ───────────────────────────────────────────────────────
        if (GUI::bHealthBar)
            DrawHealthBar(dl, boxX, sHead.y, boxH, hp, GUI::bHealthNumber);

        // ── Armor bar ────────────────────────────────────────────────────────
        if (GUI::bArmorBar) {
            int armor = Rd<int>(pPawn + pawn::m_ArmorValue);
            armor = std::max(0, std::min(100, armor));
            DrawArmorBar(dl, boxX + boxW, sHead.y, boxH, armor);
        }

        // ── Player name ──────────────────────────────────────────────────────
        if (GUI::bPlayerNames) {
            std::string name = GetPlayerName(ctrl);
            ImVec2 ts = ImGui::CalcTextSize(name.c_str());
            dl->AddText({boxX + boxW * 0.5f - ts.x * 0.5f, sHead.y - 14.f}, col, name.c_str());
        }

        // ── Weapon name + ammo ───────────────────────────────────────────────
        if (GUI::bWeaponName || GUI::bWeaponInfo) {
            uintptr_t wpn = ResolveActiveWeapon(pPawn, entList);

            if (GUI::bWeaponName && wpn) {
                uint16_t defIdx = Rd<uint16_t>(wpn + weapon::m_iItemDefinitionIndex);
                const char* wname = GetWeaponName(defIdx);
                if (wname) {
                    ImVec2 ts = ImGui::CalcTextSize(wname);
                    dl->AddText({boxX + boxW * 0.5f - ts.x * 0.5f, sFeet.y + 3.f},
                                IM_COL32(220,220,220,200), wname);
                }
            }

            if (GUI::bWeaponInfo && wpn) {
                int clip    = Rd<int>(wpn + weapon::m_iClip1);
                int reserve = Rd<int>(wpn + weapon::m_pReserveAmmo);
                if (clip >= 0 && clip <= 150 && reserve >= 0 && reserve <= 999) {
                    std::string ammo = std::to_string(clip) + "/" + std::to_string(reserve);
                    ImVec2 ts = ImGui::CalcTextSize(ammo.c_str());
                    float ammoY = GUI::bWeaponName ? sFeet.y + 16.f : sFeet.y + 3.f;
                    dl->AddText({boxX + boxW * 0.5f - ts.x * 0.5f, ammoY},
                                IM_COL32(180,180,180,200), ammo.c_str());
                }
            }
        }

        // ── Flags ─────────────────────────────────────────────────────────────
        if (GUI::bEspFlags) {
            bool hasHelmet  = Rd<uint8_t>(ctrl + controller::m_bPawnHasHelmet)  != 0;
            bool hasDefuser = Rd<uint8_t>(ctrl + controller::m_bPawnHasDefuser) != 0;
            bool hasC4 = false;
            {
                uintptr_t wpn = ResolveActiveWeapon(pPawn, entList);
                if (wpn) hasC4 = (Rd<uint16_t>(wpn + weapon::m_iItemDefinitionIndex) == 49);
            }

            float flagOffX = GUI::bArmorBar ? 16.f : 5.f;
            float flagX = boxX + boxW + flagOffX;
            float flagY = sHead.y;

            if (hasHelmet) {
                dl->AddText({flagX, flagY}, IM_COL32(230,230,230,200), "[HE]");
                flagY += 12.f;
            }
            if (hasDefuser) {
                dl->AddText({flagX, flagY}, IM_COL32(100,220,100,220), "[KIT]");
                flagY += 12.f;
            }
            if (hasC4) {
                dl->AddText({flagX, flagY}, IM_COL32(255,200,50,230), "[C4]");
                flagY += 12.f;
            }
        }

        // ── Distance ─────────────────────────────────────────────────────────
        if (GUI::bDistance) {
            float dist = (feet - localPos).Length() / 39.37f;
            std::string ds = std::to_string((int)dist) + "m";
            ImVec2 ts = ImGui::CalcTextSize(ds.c_str());
            dl->AddText({boxX + boxW * 0.5f - ts.x * 0.5f, sHead.y - 26.f},
                        IM_COL32(200,200,200,160), ds.c_str());
        }

        // ── Offscreen arrows ──────────────────────────────────────────────────
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
                        col);
                }
            }
        }
    }

    ImGui::End();
}

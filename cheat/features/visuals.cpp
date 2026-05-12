// features/visuals.cpp — no-flash implemented; others noted
#include "../pch.h"
#include "visuals.hpp"
#include "../gui/gui.h"
#include "../sdk/offsets.hpp"
#include <Windows.h>

using namespace cs2_dumper::offsets;

constexpr ptrdiff_t m_flFlashMaxAlpha  = 0x13FC;
constexpr ptrdiff_t m_flFlashDuration  = 0x1400;
constexpr ptrdiff_t m_pCameraServices  = 0x14B8; // CPlayer_CameraServices* on pawn
constexpr ptrdiff_t m_flFOVDesired     = 0x2A8;  // float in CPlayer_CameraServices

template<typename T>
static T VisRd(uintptr_t addr) {
    T r{};
    if (addr < 0x100000 || addr > 0x7FFFFFFEFFFF) return r;
    __try { r = *reinterpret_cast<T*>(addr); } __except(EXCEPTION_EXECUTE_HANDLER) { return T{}; }
    return r;
}
template<typename T>
static void VisWr(uintptr_t addr, T val) {
    if (addr < 0x100000 || addr > 0x7FFFFFFEFFFF) return;
    __try { *reinterpret_cast<T*>(addr) = val; } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

namespace Visuals {
    void Update() {
        const auto clientBase = reinterpret_cast<uintptr_t>(GetModuleHandleA("client.dll"));
        if (!clientBase) return;

        const uintptr_t localPawn = VisRd<uintptr_t>(clientBase + client_dll::dwLocalPlayerPawn);
        if (!localPawn) return;

        if (GUI::bNoFlash) {
            VisWr<float>(localPawn + m_flFlashMaxAlpha, 0.f);
            VisWr<float>(localPawn + m_flFlashDuration, 0.f);
        }

        if (GUI::fFOVChanger != 90.f) {
            const uintptr_t camSvc = VisRd<uintptr_t>(localPawn + m_pCameraServices);
            if (camSvc)
                VisWr<float>(camSvc + m_flFOVDesired, GUI::fFOVChanger);
        }

        // NO SMOKE: needs m_nSmokeEffect offset — stub until offset available
        // SKY COLOR: needs engine ConVar interface — stub
    }
}

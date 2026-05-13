// features/visuals.cpp
#include "../pch.h"
#include "visuals.hpp"
#include "../gui/gui.h"
#include "../sdk/offsets.hpp"
#include "../sdk/schemas.hpp"
#include <Windows.h>

using namespace cs2_dumper::offsets;
using namespace schemas;

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
            VisWr<float>(localPawn + pawn::m_flFlashMaxAlpha, 0.f);
            VisWr<float>(localPawn + pawn::m_flFlashDuration, 0.f);
        }

        if (GUI::fFOVChanger != 90.f) {
            const uintptr_t camSvc = VisRd<uintptr_t>(localPawn + pawn::m_pCameraServices);
            if (camSvc)
                VisWr<float>(camSvc + camera::m_flFOVDesired, GUI::fFOVChanger);
        }

        // NO SMOKE: needs m_nSmokeEffect offset — stub until offset available
        // SKY COLOR: needs engine ConVar interface — stub
    }
}

// features/movement.cpp
#include "../pch.h"
#include "movement.hpp"
#include "../gui/gui.h"
#include "../sdk/offsets.hpp"
#include "../sdk/schemas.hpp"
#include <Windows.h>

using namespace cs2_dumper::offsets;
using namespace schemas;

template<typename T>
static T MovRd(uintptr_t addr) {
    T r{};
    if (addr < 0x100000 || addr > 0x7FFFFFFEFFFF) return r;
    __try { r = *reinterpret_cast<T*>(addr); } __except(EXCEPTION_EXECUTE_HANDLER) { return T{}; }
    return r;
}

namespace Movement {
    static bool s_wasInAir = false;

    void Update() {
        const auto clientBase = reinterpret_cast<uintptr_t>(GetModuleHandleA("client.dll"));
        if (!clientBase) return;
        const uintptr_t localPawn = MovRd<uintptr_t>(clientBase + client_dll::dwLocalPlayerPawn);
        if (!localPawn) return;

        const bool onGround = (MovRd<uint32_t>(localPawn + pawn::m_fFlags) & 1) != 0;

        if (GUI::bBhop && (GetAsyncKeyState(VK_SPACE) & 0x8000)) {
            if (onGround && s_wasInAir) {
                keybd_event(VK_SPACE, 0, 0, 0);
                keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
            }
        }

        s_wasInAir = !onGround;

        // Rate-limited: at most one A/D cycle per 15ms (was firing every Present call ~300/s)
        if (GUI::bStrafe && !onGround) {
            static DWORD s_lastStrafe = 0;
            DWORD now = GetTickCount();
            if ((now - s_lastStrafe) >= 15) {
                static bool side = false;
                side = !side;
                keybd_event(side ? 'A' : 'D', 0, 0, 0);
                keybd_event(side ? 'A' : 'D', 0, KEYEVENTF_KEYUP, 0);
                s_lastStrafe = now;
            }
        }
    }
}

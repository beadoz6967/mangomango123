// features/movement.cpp — bhop implemented
#include "../pch.h"
#include "movement.hpp"
#include "../gui/gui.h"
#include "../sdk/offsets.hpp"
#include <Windows.h>

using namespace cs2_dumper::offsets;

constexpr ptrdiff_t m_fFlags = 0x3EC;

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

        const uint32_t flags    = MovRd<uint32_t>(localPawn + m_fFlags);
        const bool     onGround = (flags & 1) != 0;

        if (GUI::bBhop && (GetAsyncKeyState(VK_SPACE) & 0x8000)) {
            // Fire a jump on the frame we land so the engine sees a fresh press
            if (onGround && s_wasInAir) {
                keybd_event(VK_SPACE, 0, 0, 0);
                keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
            }
        }

        // Update state every frame regardless of SPACE, so it stays accurate
        s_wasInAir = !onGround;

        if (GUI::bStrafe && !onGround) {
            // Alternate A/D each frame while airborne — no Sleep in the render thread
            static bool side = false;
            side = !side;
            keybd_event(side ? 'A' : 'D', 0, 0, 0);
            keybd_event(side ? 'A' : 'D', 0, KEYEVENTF_KEYUP, 0);
        }
    }
}

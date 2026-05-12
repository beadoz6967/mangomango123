#include "../pch.h"
#include "visuals.hpp"
#include "../gui/gui.h"
#include <Windows.h>

namespace Visuals {
    void Update() {
        if (GUI::bNoFlash) {
            // No-flash logic would go here
        }
        if (GUI::bNoSmoke) {
            // No-smoke logic would go here
        }
        if (GUI::bSkyColor) {
            // Sky color logic would go here
        }
        if (GUI::fFOVChanger != 90.0f) {
            // FOV changer logic would go here
        }
    }
}

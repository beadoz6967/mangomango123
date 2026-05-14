#include "templeware.h"

#include "utils\logging\log.h"

#include "utils/module/module.h"

#include <iostream>

bool TempleWare::init(HWND& window, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ID3D11RenderTargetView* mainRenderTargetView) {
    Logger::Log("Initializing modules...", LogType::Info);
    modules.init();

    Logger::Log("Initializing menu...", LogType::Info);
    renderer.menu.init(window, pDevice, pContext, mainRenderTargetView);

    Logger::Log("Skipping schema bootstrap for stability; continuing with direct offsets.", LogType::Warning);

    Logger::Log("Initializing interfaces...", LogType::Info);
    if (!interfaces.init()) {
        Logger::Log("Interface init failed, stopping startup early.", LogType::Error);
        return false;
    }

    Logger::Log("Initializing visuals...", LogType::Info);
    renderer.visuals.init();

    Logger::Log("Initializing materials (disabled to prevent signature crash)...", LogType::Warning);
    // materials.init();

    Logger::Log("Initializing hooks...", LogType::Info);
    hooks.init();

    Logger::Log("Bootstrap success.", LogType::Info);
    return true;
}

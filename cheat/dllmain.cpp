#include "pch.h"
#include <windows.h>
#include "gui/hooks.h"
#include "core/config.hpp"

static HMODULE g_hModule = nullptr;

static DWORD WINAPI MainThread(LPVOID)
{
    Beep(1000, 200);

    for (int i = 0; i < 50 && !GetModuleHandleA("gameoverlayrenderer64.dll"); i++)
        Sleep(200);

    // Initialize config system
    if (!Config::Init())
    {
        Beep(400, 500);
        FreeLibraryAndExitThread(g_hModule, 1);
        return 1;
    }

    // Load saved config
    Config::Load();

    if (!Hooks::Init())
    {
        Beep(400, 500);
        FreeLibraryAndExitThread(g_hModule, 1);
        return 1;
    }

    while (!(GetAsyncKeyState(VK_END) & 1))
        Sleep(50);

    // Save config before unload
    Config::Save();
    Hooks::Shutdown();
    Beep(600, 300);
    FreeLibraryAndExitThread(g_hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);
        HANDLE hThread = CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
        if (hThread)
            CloseHandle(hThread);
    }
    return TRUE;
}

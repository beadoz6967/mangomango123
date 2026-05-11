#include "pch.h"
#include <windows.h>
#include "gui/hooks.h"

static HMODULE g_hModule = nullptr;

static DWORD WINAPI MainThread(LPVOID)
{
    Beep(1000, 200);

    if (!Hooks::Init())
    {
        Beep(400, 500);
        FreeLibraryAndExitThread(g_hModule, 1);
        return 1;
    }

    while (!(GetAsyncKeyState(VK_END) & 1))
        Sleep(50);

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

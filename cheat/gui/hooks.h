#pragma once
#include <d3d11.h>
#include <windows.h>

namespace Hooks
{
    using Present_t  = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT);
    using WndProc_t  = LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM);

    inline Present_t  oPresent = nullptr;
    inline WndProc_t  oWndProc = nullptr;
    inline HWND       g_Hwnd   = nullptr;

    bool   Init();
    void   Shutdown();

    HRESULT WINAPI hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
    LRESULT WINAPI hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}

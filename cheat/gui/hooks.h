#pragma once
#include <d3d11.h>
#include <windows.h>

namespace Hooks
{
    using Present_t        = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT);
    using ResizeBuffers_t  = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
    using WndProc_t        = LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM);

    inline Present_t        oPresent        = nullptr;
    inline ResizeBuffers_t  oResizeBuffers  = nullptr;
    inline WndProc_t        oWndProc        = nullptr;
    inline HWND             g_Hwnd          = nullptr;

    bool   Init();
    void   Shutdown();

    HRESULT WINAPI hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
    HRESULT WINAPI hkResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
    LRESULT WINAPI hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}

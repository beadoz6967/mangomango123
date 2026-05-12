#include "hooks.h"
#include "gui.h"
#include "../imgui/imgui.h"
#include "../imgui/backends/imgui_impl_win32.h"
#include "../imgui/backends/imgui_impl_dx11.h"
#include <MinHook.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Hooks
{
    static ID3D11Device*           g_pd3dDevice  = nullptr;
    static ID3D11DeviceContext*    g_pd3dContext  = nullptr;
    static ID3D11RenderTargetView* g_mainRTV      = nullptr;
    static bool                    g_bInitialized = false;

    static void CreateRTV(IDXGISwapChain* pSwapChain)
    {
        ID3D11Texture2D* pBack = nullptr;
        if (SUCCEEDED(pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBack))))
        {
            g_pd3dDevice->CreateRenderTargetView(pBack, nullptr, &g_mainRTV);
            pBack->Release();
        }
    }

    HRESULT WINAPI hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
    {
        if (!g_bInitialized)
        {
            if (FAILED(pSwapChain->GetDevice(IID_PPV_ARGS(&g_pd3dDevice))))
                return oPresent(pSwapChain, SyncInterval, Flags);

            g_pd3dDevice->GetImmediateContext(&g_pd3dContext);

            DXGI_SWAP_CHAIN_DESC sd{};
            pSwapChain->GetDesc(&sd);
            g_Hwnd = sd.OutputWindow;

            CreateRTV(pSwapChain);

            oWndProc = reinterpret_cast<WndProc_t>(
                SetWindowLongPtrW(g_Hwnd, GWLP_WNDPROC,
                    reinterpret_cast<LONG_PTR>(hkWndProc)));

            ImGui::CreateContext();
            GUI::ApplyStyle();
            ImGui_ImplWin32_Init(g_Hwnd);
            ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dContext);

            g_bInitialized = true;
        }

        if (GetAsyncKeyState(VK_INSERT) & 1)
            GUI::g_Open = !GUI::g_Open;

        if (GUI::g_Open)
        {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            GUI::Render();

            ImGui::Render();
            g_pd3dContext->OMSetRenderTargets(1, &g_mainRTV, nullptr);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }

        return oPresent(pSwapChain, SyncInterval, Flags);
    }

    LRESULT WINAPI hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (GUI::g_Open && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
            return 1L;

        return CallWindowProcW(oWndProc, hWnd, uMsg, wParam, lParam);
    }

    bool Init()
    {
        WNDCLASSEXW wc{ sizeof(wc) };
        wc.lpfnWndProc   = DefWindowProcW;
        wc.hInstance     = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"DX11Dummy_cs2c";
        RegisterClassExW(&wc);

        HWND hDummy = CreateWindowExW(0, wc.lpszClassName, L"",
            WS_OVERLAPPED, 0, 0, 1, 1, nullptr, nullptr, wc.hInstance, nullptr);
        if (!hDummy)
        {
            UnregisterClassW(wc.lpszClassName, wc.hInstance);
            return false;
        }

        DXGI_SWAP_CHAIN_DESC scd{};
        scd.BufferCount        = 1;
        scd.BufferDesc.Format  = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow       = hDummy;
        scd.SampleDesc.Count   = 1;
        scd.Windowed           = TRUE;
        scd.SwapEffect         = DXGI_SWAP_EFFECT_DISCARD;

        IDXGISwapChain*  pDummySC  = nullptr;
        ID3D11Device*    pDummyDev = nullptr;
        D3D_FEATURE_LEVEL fl       = D3D_FEATURE_LEVEL_11_0;

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            &fl, 1, D3D11_SDK_VERSION,
            &scd, &pDummySC, &pDummyDev,
            nullptr, nullptr);

        if (FAILED(hr))
        {
            DestroyWindow(hDummy);
            UnregisterClassW(wc.lpszClassName, wc.hInstance);
            return false;
        }

        // vtable index 8 = IDXGISwapChain::Present
        void** vtable = *reinterpret_cast<void***>(pDummySC);
        void*  pPresentTarget = vtable[8];

        pDummySC->Release();
        pDummyDev->Release();
        DestroyWindow(hDummy);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);

        if (MH_Initialize() != MH_OK)
            return false;

        if (MH_CreateHook(pPresentTarget, hkPresent,
                reinterpret_cast<void**>(&oPresent)) != MH_OK)
            return false;

        return MH_EnableHook(pPresentTarget) == MH_OK;
    }

    void Shutdown()
    {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();

        if (oWndProc && g_Hwnd)
            SetWindowLongPtrW(g_Hwnd, GWLP_WNDPROC,
                reinterpret_cast<LONG_PTR>(oWndProc));

        if (g_bInitialized)
        {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            g_bInitialized = false;
        }

        if (g_mainRTV)    { g_mainRTV->Release();    g_mainRTV    = nullptr; }
        if (g_pd3dContext) { g_pd3dContext->Release(); g_pd3dContext = nullptr; }
        if (g_pd3dDevice)  { g_pd3dDevice->Release();  g_pd3dDevice  = nullptr; }
    }
}

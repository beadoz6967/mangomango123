#include "debug/debug.h"

#include "templeware\utils\logging\log.h"

#include "includes.h"
#include "templeware/templeware.h"
#include "templeware/renderer/icons.h"

#include "../external/kiero/minhook/include/MinHook.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

TempleWare templeWare;

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;

static LONG ReportPresentException(EXCEPTION_POINTERS* exceptionPointers) {
    if (exceptionPointers && exceptionPointers->ExceptionRecord) {
        Logger::Logf(LogType::Error,
                     "Unhandled exception in hkPresent: code=0x%08lX address=%p",
                     exceptionPointers->ExceptionRecord->ExceptionCode,
                     exceptionPointers->ExceptionRecord->ExceptionAddress);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

static LONG WINAPI CrashFilter(EXCEPTION_POINTERS* exceptionPointers) {
    if (exceptionPointers && exceptionPointers->ExceptionRecord) {
        Logger::Logf(LogType::Error,
                     "Unhandled process exception: code=0x%08lX address=%p",
                     exceptionPointers->ExceptionRecord->ExceptionCode,
                     exceptionPointers->ExceptionRecord->ExceptionAddress);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
        return true;
    }

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool init = false;
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    __try {
        if (!init) {
            if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice))) {
                Logger::Logf(LogType::Info, "hkPresent bootstrap started: device=%p", pDevice);
                pDevice->GetImmediateContext(&pContext);

                if (!pContext) {
                    Logger::Log("hkPresent failed: immediate context is null", LogType::Error);
                    pDevice->Release();
                    pDevice = nullptr;
                    return oPresent(pSwapChain, SyncInterval, Flags);
                }

                DXGI_SWAP_CHAIN_DESC sd{};
                if (FAILED(pSwapChain->GetDesc(&sd))) {
                    Logger::Log("hkPresent failed: GetDesc failed", LogType::Error);
                    pContext->Release();
                    pContext = nullptr;
                    pDevice->Release();
                    pDevice = nullptr;
                    return oPresent(pSwapChain, SyncInterval, Flags);
                }

                window = sd.OutputWindow;
                ID3D11Texture2D* pBackBuffer = nullptr;
                if (FAILED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer)) || !pBackBuffer) {
                    Logger::Log("hkPresent failed: back buffer unavailable", LogType::Error);
                    pContext->Release();
                    pContext = nullptr;
                    pDevice->Release();
                    pDevice = nullptr;
                    return oPresent(pSwapChain, SyncInterval, Flags);
                }

                if (FAILED(pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView)) || !mainRenderTargetView) {
                    Logger::Log("hkPresent failed: CreateRenderTargetView failed", LogType::Error);
                    pBackBuffer->Release();
                    pContext->Release();
                    pContext = nullptr;
                    pDevice->Release();
                    pDevice = nullptr;
                    return oPresent(pSwapChain, SyncInterval, Flags);
                }

                pBackBuffer->Release();
                oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
#ifdef TEMPLEDEBUG
            initDebug();
#endif

                if (templeWare.init(window, pDevice, pContext, mainRenderTargetView)) {
                    Logger::Log("TempleWare bootstrap complete", LogType::Info);
                    init = true;
                } else {
                    Logger::Log("TempleWare bootstrap failed before hook became active", LogType::Error);
                }
            }
            else {
                Logger::Log("hkPresent failed: GetDevice failed", LogType::Error);
                return oPresent(pSwapChain, SyncInterval, Flags);
            }
        }

        ImFontConfig imIconsConfig;
        imIconsConfig.RasterizerMultiply = 1.2f;

        constexpr ImWchar wIconRanges[] =
        {
            0xE000, 0xF8FF, // Private Use Area
            0
        };

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (GetAsyncKeyState(VK_INSERT) & 1) {
            Logger::Log("INSERT pressed: toggling menu", LogType::Info);
            templeWare.renderer.menu.toggleMenu();
        }

        templeWare.renderer.menu.render();
        templeWare.renderer.hud.render();

        // Always call esp() to allow individual components to be rendered
        templeWare.renderer.visuals.esp();

        ImGui::Render();
        pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        return oPresent(pSwapChain, SyncInterval, Flags);
    }
    __except (ReportPresentException(GetExceptionInformation())) {
        return oPresent ? oPresent(pSwapChain, SyncInterval, Flags) : S_OK;
    }
}

void init_console() {
    Logger::Init();

    if (::AllocConsole()) {
        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);  // Redirect stdout to console
        freopen_s(&f, "CONOUT$", "w", stderr);  // Redirect stderr to console
        ::SetConsoleTitleW(L"ENI");

        Logger::Log("Console allocated", LogType::Info);

        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        printf(R"(
   ▄████████ ███▄▄▄▄      ▄█  
  ███    ███ ███▀▀▀██▄   ███  
  ███    █▀  ███   ███   ███▌ 
 ▄███▄▄▄     ███   ███   ███▌ 
▀▀███▀▀▀     ███   ███   ███▌ 
  ███    █▄  ███   ███   ███  
  ███    ███ ███   ███   ███  
  ██████████  ▀█   █▀    █▀   
)");

        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

        printf("[");
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        printf("+");
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        printf("] ENI allocated successfully!\n");
    } else {
        Logger::Log("Failed to allocate console", LogType::Warning);
    }
}
DWORD WINAPI MainThread(LPVOID lpReserved)
{
    SetUnhandledExceptionFilter(CrashFilter);
    init_console();

    Logger::Log("Main thread started", LogType::Info);

    // hook hkPresent and init cheat
    Logger::Log("Waiting for kiero D3D11 initialization", LogType::Info);
    while (kiero::init(kiero::RenderType::D3D11) != kiero::Status::Success) {
        Sleep(100);
    }
    Logger::Log("kiero initialized", LogType::Info);
    kiero::bind(8, (void**)&oPresent, hkPresent);
    Logger::Log("Present hook bound", LogType::Info);

    while (!GetAsyncKeyState(VK_F4)) {
        Sleep(100);
    }

    Logger::Log("F4 pressed, shutting down", LogType::Warning);

    if (oWndProc != nullptr)
    {
        // restore wnd proc
        SetWindowLongPtrW(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(oWndProc));

        // invalidate old wnd proc
        oWndProc = nullptr;
    }

    kiero::shutdown();

    // destroy minhook
    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    // free allocated memory for console
    ::FreeConsole();

    // close console window
    if (const HWND hConsoleWindow = ::GetConsoleWindow(); hConsoleWindow != nullptr)
        ::PostMessageW(hConsoleWindow, WM_CLOSE, 0U, 0L);

    fclose(stdout);
    fclose(stderr);

    Logger::Log("Shutdown complete", LogType::Info);
    Logger::Shutdown();

    // close thread
    FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(lpReserved), EXIT_SUCCESS);

    return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hMod);
        CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        kiero::shutdown();
        break;
    }
    return TRUE;
}

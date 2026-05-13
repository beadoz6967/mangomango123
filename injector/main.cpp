// injector/main.cpp
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// ANSI color helpers
#define C_RESET  "\033[0m"
#define C_RED    "\033[31m"
#define C_GREEN  "\033[32m"
#define C_YELLOW "\033[33m"
#define C_CYAN   "\033[36m"
#define C_BOLD   "\033[1m"

static void EnableAnsi() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

static std::string WinError() {
    DWORD id = GetLastError();
    if (!id) return "No error";
    LPSTR buf = nullptr;
    size_t sz = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, 0, nullptr);
    std::string s(buf, sz);
    LocalFree(buf);
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
    return s + " (" + std::to_string(id) + ")";
}

static int GetDllBitness(const char* path) {
    HANDLE hf = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hf == INVALID_HANDLE_VALUE) return 0;
    IMAGE_DOS_HEADER dh{}; DWORD br = 0;
    ReadFile(hf, &dh, sizeof(dh), &br, nullptr);
    if (dh.e_magic != IMAGE_DOS_SIGNATURE) { CloseHandle(hf); return 0; }
    SetFilePointer(hf, dh.e_lfanew, nullptr, FILE_BEGIN);
    DWORD sig = 0; ReadFile(hf, &sig, sizeof(sig), &br, nullptr);
    if (sig != IMAGE_NT_SIGNATURE) { CloseHandle(hf); return 0; }
    IMAGE_FILE_HEADER fh{}; ReadFile(hf, &fh, sizeof(fh), &br, nullptr);
    CloseHandle(hf);
    if (fh.Machine == IMAGE_FILE_MACHINE_AMD64) return 64;
    if (fh.Machine == IMAGE_FILE_MACHINE_I386)  return 32;
    return 0;
}

// Poll process list every 500ms up to timeoutSec. Returns PID or 0.
static DWORD WaitForProcess(const char* name, int timeoutSec) {
    for (int i = 0, ticks = timeoutSec * 2; i < ticks; i++) {
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 pe{ sizeof(pe) };
            if (Process32First(snap, &pe)) {
                do {
                    if (_stricmp(pe.szExeFile, name) == 0) {
                        CloseHandle(snap);
                        return pe.th32ProcessID;
                    }
                } while (Process32Next(snap, &pe));
            }
            CloseHandle(snap);
        }
        Sleep(500);
        if (i % 4 == 3) { std::cout << "." << std::flush; }
    }
    return 0;
}

// Returns true if module (by filename) is loaded in target process.
static bool IsModuleLoaded(DWORD pid, const char* dllPath) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE) return false;
    const std::string target = fs::path(dllPath).filename().string();
    MODULEENTRY32 me{ sizeof(me) };
    bool found = false;
    if (Module32First(snap, &me)) {
        do {
            if (_stricmp(me.szModule, target.c_str()) == 0) { found = true; break; }
        } while (Module32Next(snap, &me));
    }
    CloseHandle(snap);
    return found;
}

// Wait up to timeoutSec for a named module to appear in target process.
static bool WaitForModule(DWORD pid, const char* modName, int timeoutSec) {
    for (int i = 0, ticks = timeoutSec * 2; i < ticks; i++) {
        if (IsModuleLoaded(pid, modName)) return true;
        Sleep(500);
    }
    return false;
}

int main(int argc, char* argv[]) {
    EnableAnsi();
    SetConsoleTitleA("LitWare Injector");

    std::cout << C_BOLD C_CYAN
              << "  +-----------------------------+\n"
              << "  |    LitWare CS2 Injector     |\n"
              << "  +-----------------------------+\n"
              << C_RESET "\n";

    // ── Resolve DLL path ──────────────────────────────────────────────────────
    std::string dllPath;
    if (argc >= 2) {
        dllPath = argv[1];
    } else {
        char self[MAX_PATH]{};
        GetModuleFileNameA(nullptr, self, MAX_PATH);
        fs::path candidate = fs::path(self).parent_path() / "cheat.dll";
        if (!fs::exists(candidate)) {
            std::cerr << C_RED "[-] cheat.dll not found next to injector.\n"
                               "    Usage: injector.exe [path\\to\\cheat.dll]\n" C_RESET;
            std::cin.get(); return 1;
        }
        dllPath = candidate.string();
    }
    std::cout << C_CYAN "[*] DLL : " << dllPath << C_RESET "\n";

    if (GetFileAttributesA(dllPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        std::cerr << C_RED "[-] File not found: " << dllPath << C_RESET "\n";
        std::cin.get(); return 1;
    }

    int bits = GetDllBitness(dllPath.c_str());
    if (bits != 64) {
        std::cerr << C_RED << (bits == 32 ? "[-] DLL is 32-bit — rebuild as x64|Release.\n"
                                          : "[-] Cannot read DLL PE headers.\n") << C_RESET;
        std::cin.get(); return 1;
    }
    std::cout << C_GREEN "[+] DLL validated (x64).\n" C_RESET;

    // ── Wait for CS2 ─────────────────────────────────────────────────────────
    std::cout << C_YELLOW "[~] Waiting for cs2.exe (60s)" C_RESET;
    DWORD pid = WaitForProcess("cs2.exe", 60);
    if (!pid) {
        std::cerr << "\n" C_RED "[-] cs2.exe not found after 60 seconds.\n" C_RESET;
        std::cin.get(); return 1;
    }
    std::cout << "\n" C_GREEN "[+] cs2.exe PID: " << std::dec << pid << C_RESET "\n";

    // ── Already injected? ─────────────────────────────────────────────────────
    if (IsModuleLoaded(pid, dllPath.c_str())) {
        std::cout << C_YELLOW "[!] Already injected — nothing to do.\n" C_RESET;
        std::cin.get(); return 0;
    }

    // ── Wait for Steam overlay (signals game is fully loaded) ─────────────────
    std::cout << C_YELLOW "[~] Waiting for Steam overlay..." C_RESET;
    if (!WaitForModule(pid, "gameoverlayrenderer64.dll", 30))
        std::cout << "\n" C_YELLOW "[!] Overlay not detected — injecting anyway.\n" C_RESET;
    else
        std::cout << "\n" C_GREEN "[+] Overlay ready.\n" C_RESET;

    // ── Open process (minimal rights) ─────────────────────────────────────────
    constexpr DWORD NEEDED = PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
                             PROCESS_VM_OPERATION  | PROCESS_VM_WRITE | PROCESS_VM_READ;
    HANDLE hProc = OpenProcess(NEEDED, FALSE, pid);
    if (!hProc) {
        std::cerr << C_RED "[-] OpenProcess failed (run as Admin): " << WinError() << C_RESET "\n";
        std::cin.get(); return 1;
    }

    // ── Write DLL path into target ────────────────────────────────────────────
    size_t pathLen = dllPath.size() + 1;
    void* pMem = VirtualAllocEx(hProc, nullptr, pathLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pMem) {
        std::cerr << C_RED "[-] VirtualAllocEx failed: " << WinError() << C_RESET "\n";
        CloseHandle(hProc); std::cin.get(); return 1;
    }
    if (!WriteProcessMemory(hProc, pMem, dllPath.c_str(), pathLen, nullptr)) {
        std::cerr << C_RED "[-] WriteProcessMemory failed: " << WinError() << C_RESET "\n";
        VirtualFreeEx(hProc, pMem, 0, MEM_RELEASE); CloseHandle(hProc); std::cin.get(); return 1;
    }

    // ── CreateRemoteThread → LoadLibraryA ────────────────────────────────────
    // Resolve LoadLibraryA at runtime so address is valid in target address space.
    auto loadLib = reinterpret_cast<LPTHREAD_START_ROUTINE>(
        GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"));

    HANDLE hThread = CreateRemoteThread(hProc, nullptr, 0, loadLib, pMem, 0, nullptr);
    if (!hThread) {
        std::cerr << C_RED "[-] CreateRemoteThread failed: " << WinError() << C_RESET "\n";
        VirtualFreeEx(hProc, pMem, 0, MEM_RELEASE); CloseHandle(hProc); std::cin.get(); return 1;
    }

    std::cout << C_YELLOW "[~] Loading DLL..." C_RESET "\n";
    WaitForSingleObject(hThread, 10'000);  // 10s timeout

    DWORD exitCode = 0;
    GetExitCodeThread(hThread, &exitCode);
    CloseHandle(hThread);
    VirtualFreeEx(hProc, pMem, 0, MEM_RELEASE);
    CloseHandle(hProc);

    // ── Verify via module list ────────────────────────────────────────────────
    Sleep(300);
    if (IsModuleLoaded(pid, dllPath.c_str())) {
        std::cout << C_BOLD C_GREEN "[+] Injected successfully!\n" C_RESET;
        std::cout << C_GREEN "[+] Module base: 0x" << std::hex << exitCode << C_RESET "\n";
    } else if (exitCode) {
        std::cout << C_YELLOW "[?] Thread returned 0x" << std::hex << exitCode
                  << " but module not in list (may still load).\n" C_RESET;
    } else {
        std::cerr << C_RED "[-] LoadLibraryA returned NULL — DLL failed to load.\n"
                           "    Check: VC++ runtime installed, DllMain not crashing, x64 target.\n"
                  << C_RESET;
        std::cin.get(); return 1;
    }

    std::cout << "\nPress Enter to exit...\n";
    std::cin.get();
    return 0;
}

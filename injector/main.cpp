// injector/main.cpp — FIXED: no longer hardcoded to absolute path
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

std::string GetLastErrorAsString() {
    DWORD id = ::GetLastError();
    if (!id) return "No error";
    LPSTR buf = nullptr;
    size_t sz = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, 0, nullptr);
    std::string msg(buf, sz);
    LocalFree(buf);
    return msg;
}

DWORD FindProcessId(const char* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32 pe{ sizeof(pe) };
    if (Process32First(snap, &pe)) {
        do { if (_stricmp(pe.szExeFile, name) == 0) { CloseHandle(snap); return pe.th32ProcessID; } }
        while (Process32Next(snap, &pe));
    }
    CloseHandle(snap);
    return 0;
}

int GetDllBitness(const char* path) {
    HANDLE hf = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
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

int main(int argc, char* argv[]) {
    // FIXED: resolve DLL path relative to injector's own directory, not hardcoded
    std::string dllPath;

    if (argc >= 2) {
        dllPath = argv[1];
    } else {
        char selfPath[MAX_PATH]{};
        GetModuleFileNameA(nullptr, selfPath, MAX_PATH);
        fs::path candidate = fs::path(selfPath).parent_path() / "cheat.dll";
        if (!fs::exists(candidate)) {
            std::cerr << "[-] cheat.dll not found next to injector.\n";
            std::cerr << "    Usage: injector.exe [path\\to\\cheat.dll]\n";
            system("pause"); return 1;
        }
        dllPath = candidate.string();
    }

    std::cout << "[*] DLL path: " << dllPath << "\n";

    if (GetFileAttributesA(dllPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        std::cerr << "[-] DLL not found: " << dllPath << "\n";
        system("pause"); return 1;
    }
    std::cout << "[+] DLL found.\n";

    if (int bits = GetDllBitness(dllPath.c_str()); bits != 64) {
        std::cerr << (bits == 32 ? "[-] DLL is 32-bit — rebuild as x64|Release.\n"
                                 : "[-] Could not read DLL PE headers.\n");
        system("pause"); return 1;
    }
    std::cout << "[+] DLL is 64-bit.\n";

    DWORD pid = FindProcessId("cs2.exe");
    if (!pid) { std::cerr << "[-] cs2.exe not running.\n"; system("pause"); return 1; }
    std::cout << "[+] cs2.exe PID: " << pid << "\n";

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProc) { std::cerr << "[-] OpenProcess failed (run as Admin): " << GetLastErrorAsString() << "\n"; system("pause"); return 1; }

    void* pMem = VirtualAllocEx(hProc, nullptr, dllPath.size() + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pMem) { std::cerr << "[-] VirtualAllocEx failed: " << GetLastErrorAsString() << "\n"; CloseHandle(hProc); system("pause"); return 1; }

    if (!WriteProcessMemory(hProc, pMem, dllPath.c_str(), dllPath.size() + 1, nullptr)) {
        std::cerr << "[-] WriteProcessMemory failed: " << GetLastErrorAsString() << "\n";
        VirtualFreeEx(hProc, pMem, 0, MEM_RELEASE); CloseHandle(hProc); system("pause"); return 1;
    }

    HANDLE hThread = CreateRemoteThread(hProc, nullptr, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, pMem, 0, nullptr);
    if (!hThread) {
        std::cerr << "[-] CreateRemoteThread failed: " << GetLastErrorAsString() << "\n";
        VirtualFreeEx(hProc, pMem, 0, MEM_RELEASE); CloseHandle(hProc); system("pause"); return 1;
    }

    WaitForSingleObject(hThread, INFINITE);
    DWORD exit = 0; GetExitCodeThread(hThread, &exit);

    if (!exit) {
        std::cerr << "[-] LoadLibraryA returned 0 — DLL failed inside cs2.exe.\n";
        std::cerr << "    Check: VC++ runtime installed, DllMain not crashing, x64 target.\n";
    } else {
        std::cout << "[+] Injected! Module base: 0x" << std::hex << exit << "\n";
    }

    VirtualFreeEx(hProc, pMem, 0, MEM_RELEASE);
    CloseHandle(hThread); CloseHandle(hProc);
    system("pause"); return 0;
}

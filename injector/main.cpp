#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <sstream>

const char* dllPath = "C:\\Users\\fofol\\Documents\\CHEATYcs2\\cs2cheat\\x64\\Release\\cheat.dll";

std::string GetLastErrorAsString()
{
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) return "No error";

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}

DWORD FindProcessId(const char* processName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, processName) == 0) {
                CloseHandle(hSnapshot);
                return pe.th32ProcessID;
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return 0;
}

int GetDllBitness(const char* path)
{
    HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return 0;

    IMAGE_DOS_HEADER dosHeader{};
    DWORD bytesRead = 0;
    ReadFile(hFile, &dosHeader, sizeof(dosHeader), &bytesRead, nullptr);
    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) { CloseHandle(hFile); return 0; }

    SetFilePointer(hFile, dosHeader.e_lfanew, nullptr, FILE_BEGIN);
    DWORD ntSignature = 0;
    ReadFile(hFile, &ntSignature, sizeof(ntSignature), &bytesRead, nullptr);
    if (ntSignature != IMAGE_NT_SIGNATURE) { CloseHandle(hFile); return 0; }

    IMAGE_FILE_HEADER fileHeader{};
    ReadFile(hFile, &fileHeader, sizeof(fileHeader), &bytesRead, nullptr);
    CloseHandle(hFile);

    if (fileHeader.Machine == IMAGE_FILE_MACHINE_AMD64) return 64;
    if (fileHeader.Machine == IMAGE_FILE_MACHINE_I386)  return 32;
    return 0;
}

int main()
{
    // --- DLL exists? ---
    if (GetFileAttributesA(dllPath) == INVALID_FILE_ATTRIBUTES) {
        std::cerr << "[-] DLL not found: " << dllPath << "\n";
        std::cerr << "    Rebuild the cheat project as x64|Release.\n";
        system("pause");
        return 1;
    }
    std::cout << "[+] DLL found on disk.\n";

    // --- 64-bit? ---
    int bitness = GetDllBitness(dllPath);
    if (bitness == 32) {
        std::cerr << "[-] DLL is 32-bit. CS2 is 64-bit. Rebuild as x64|Release.\n";
        system("pause");
        return 1;
    }
    if (bitness == 0) {
        std::cerr << "[-] Could not read DLL PE headers. File may be corrupt.\n";
        system("pause");
        return 1;
    }
    std::cout << "[+] DLL is 64-bit.\n";

    // --- Find process ---
    DWORD processId = FindProcessId("cs2.exe");
    if (!processId) {
        std::cerr << "[-] cs2.exe not found. Is the game running?\n";
        system("pause");
        return 1;
    }
    std::cout << "[+] Found cs2.exe PID: " << processId << "\n";

    // --- Open process ---
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess) {
        std::cerr << "[-] OpenProcess failed. Run as Administrator.\n";
        std::cerr << "    Error: " << GetLastErrorAsString() << "\n";
        system("pause");
        return 1;
    }

    // --- Allocate + write path ---
    void* pDllPath = VirtualAllocEx(hProcess, nullptr, strlen(dllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pDllPath) {
        std::cerr << "[-] VirtualAllocEx failed: " << GetLastErrorAsString() << "\n";
        CloseHandle(hProcess);
        system("pause");
        return 1;
    }

    if (!WriteProcessMemory(hProcess, pDllPath, dllPath, strlen(dllPath) + 1, nullptr)) {
        std::cerr << "[-] WriteProcessMemory failed: " << GetLastErrorAsString() << "\n";
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        system("pause");
        return 1;
    }

    // --- Inject ---
    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0,
        (LPTHREAD_START_ROUTINE)LoadLibraryA, pDllPath, 0, nullptr);
    if (!hThread) {
        std::cerr << "[-] CreateRemoteThread failed: " << GetLastErrorAsString() << "\n";
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        system("pause");
        return 1;
    }

    WaitForSingleObject(hThread, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeThread(hThread, &exitCode);

    if (exitCode == 0) {
        std::cerr << "[-] LoadLibraryA returned 0 — DLL failed to load inside cs2.exe.\n";
        std::cerr << "    - Run Dependencies.exe on your cheat.dll to find missing imports\n";
        std::cerr << "    - Make sure VC++ runtime is installed\n";
        std::cerr << "    - Check your DllMain isn't crashing on attach\n";
    }
    else {
        std::cout << "[+] Injected! Module base: 0x" << std::hex << exitCode << "\n";
    }

    VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    system("pause");
    return 0;
}
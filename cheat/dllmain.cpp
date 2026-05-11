#include "pch.h"
#include <windows.h>
#include <cstddef>
#include <cstdio>
#include "sdk/offsets.hpp"
#include "sdk/client_dll.hpp"
#include <string>
#include <sstream>
#include <shlobj.h> // For SHGetFolderPath

void WriteLog(const char* text) {
    char tempPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, tempPath))) {
        strcat_s(tempPath, "\\cheat_log.txt");
        HANDLE hFile = CreateFileA(tempPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            SetFilePointer(hFile, 0, NULL, FILE_END);
            DWORD written;
            WriteFile(hFile, text, (DWORD)strlen(text), &written, NULL); // Explicit cast to DWORD
            CloseHandle(hFile);
        }
    }
}

std::string GetLastErrorAsString() {
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return "No error";
    }

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
    WriteLog("[+] MainThread started\n");

    // Get the base address of client.dll
    HMODULE hClient = GetModuleHandleA("client.dll");
    if (!hClient)
    {
        WriteLog("[-] Failed to find client.dll!\n");
        FreeLibraryAndExitThread((HMODULE)lpReserved, 0);
        return 0;
    }

    char buf[128];
    sprintf_s(buf, "[+] Found client.dll at: %p\n", hClient);
    WriteLog(buf);

    // Read the local player pawn pointer safely
    uintptr_t localPlayerAddress = *(uintptr_t*)((uintptr_t)hClient + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
    if (!localPlayerAddress)
    {
        WriteLog("[-] Failed to find local player (Address is null)!\n");
        FreeLibraryAndExitThread((HMODULE)lpReserved, 0);
        return 0;
    }

    sprintf_s(buf, "[+] Found localPlayerAddress at: 0x%llx\n", localPlayerAddress);
    WriteLog(buf);

    // Checking if we actually read a valid pointer
    if (IsBadReadPtr((void*)localPlayerAddress, sizeof(uintptr_t)))
    {
        WriteLog("[-] Invalid local player pointer read!\n");
        FreeLibraryAndExitThread((HMODULE)lpReserved, 0);
        return 0;
    }

    // Replace the hardcoded m_iHealth offset with the correct value
    constexpr std::ptrdiff_t m_iHealth = 0x34C;

    int health = 0;
    // Attempting safely reading the health
    __try {
        health = *(int*)(localPlayerAddress + m_iHealth);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        WriteLog("[-] Access violation reading health!\n");
        FreeLibraryAndExitThread((HMODULE)lpReserved, 0);
        return 0;
    }

    sprintf_s(buf, "[+] Player Health: %d\n", health);
    WriteLog(buf);

    FreeLibraryAndExitThread((HMODULE)lpReserved, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        MessageBoxA(NULL, "DLL_PROCESS_ATTACH triggered!", "DLL Loaded", MB_OK | MB_ICONINFORMATION);
        WriteLog("[+] DLL_PROCESS_ATTACH triggered!\n");
        Beep(750, 1000); // 750Hz frequency, 1 second duration
        WriteLog("[+] Beep executed successfully.\n");

        DisableThreadLibraryCalls(hModule);
        WriteLog("[+] Disabled thread library calls.\n");

        HANDLE hThread = CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
        if (!hThread) {
            std::ostringstream oss;
            oss << "[-] Failed to create MainThread! Error: " << GetLastErrorAsString() << "\n";
            WriteLog(oss.str().c_str());
        } else {
            WriteLog("[+] MainThread created successfully.\n");
            CloseHandle(hThread);
        }
        break;
    }
    case DLL_PROCESS_DETACH:
        WriteLog("[+] DLL_PROCESS_DETACH triggered!\n");
        break;
    default:
        WriteLog("[+] Unknown reason for call in DllMain.\n");
        break;
    }
    return TRUE;
}

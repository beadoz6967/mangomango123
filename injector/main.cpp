#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <sstream>

const char* dllPath = "C:\\Users\\fofol\\Documents\\CHEATYcs2\\cs2cheat\\x64\\Release\\cheat.dll";

DWORD FindProcessId(const char* processName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        system("pause");
        return 0;
    }

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
    system("pause");
    return 0;
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

int main()
{
    DWORD processId = FindProcessId("cs2.exe");
    if (!processId) {
        std::cerr << "cs2.exe not found!\n";
        system("pause");
        return 1;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess) {
        std::cerr << "Failed to open process. Error: " << GetLastErrorAsString() << "\n";
        system("pause");
        return 1;
    }

    void* pDllPath = VirtualAllocEx(hProcess, nullptr, strlen(dllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pDllPath) {
        std::cerr << "Failed to allocate memory in target process. Error: " << GetLastErrorAsString() << "\n";
        CloseHandle(hProcess);
        system("pause");
        return 1;
    }

    if (!WriteProcessMemory(hProcess, pDllPath, dllPath, strlen(dllPath) + 1, nullptr)) {
        std::cerr << "Failed to write to process memory. Error: " << GetLastErrorAsString() << "\n";
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        system("pause");
        return 1;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, pDllPath, 0, nullptr);
    if (!hThread) {
        std::cerr << "Failed to create remote thread. Error: " << GetLastErrorAsString() << "\n";
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        system("pause");
        return 1;
    }

    WaitForSingleObject(hThread, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeThread(hThread, &exitCode);

    std::cout << "DLL injected successfully!\n";
    std::cout << "Remote thread (LoadLibraryA) returned: 0x" << std::hex << exitCode << std::dec << "\n";
    if (exitCode == 0) {
        std::cerr << "WARNING: LoadLibraryA returned 0. The DLL failed to load in the target process (likely a missing dependency or architecture mismatch).\n";
    }

    VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    system("pause");
    return 0;
}
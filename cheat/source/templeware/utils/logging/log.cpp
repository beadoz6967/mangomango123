#include "log.h"
#include <Windows.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>

namespace {
    std::mutex g_logMutex;
    std::ofstream g_logFile;
    std::filesystem::path g_logPath;
    bool g_initialized = false;

    HMODULE GetSelfModule() {
        HMODULE module = nullptr;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           reinterpret_cast<LPCSTR>(&GetSelfModule),
                           &module);
        return module;
    }

    std::string GetTimestamp() {
        SYSTEMTIME time{};
        GetLocalTime(&time);

        char buffer[64]{};
        std::snprintf(buffer, sizeof(buffer), "%04u-%02u-%02u %02u:%02u:%02u.%03u",
                      time.wYear, time.wMonth, time.wDay,
                      time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
        return buffer;
    }

    const char* GetPrefix(LogType logType) {
        switch (logType) {
            case LogType::Warning: return "[?]";
            case LogType::Error:   return "[!]";
            case LogType::None:    return "   ";
            case LogType::Info:
            default:               return "[*]";
        }
    }

    void WriteLine(const std::string& line) {
        std::lock_guard<std::mutex> lock(g_logMutex);

        if (g_logFile.is_open()) {
            g_logFile << line << '\n';
            g_logFile.flush();
        }

        OutputDebugStringA((line + "\n").c_str());
    }
}

void Logger::Init() {
    std::lock_guard<std::mutex> lock(g_logMutex);
    if (g_initialized) {
        return;
    }

    char modulePath[MAX_PATH]{};
    if (HMODULE module = GetSelfModule(); module != nullptr && GetModuleFileNameA(module, modulePath, MAX_PATH) != 0) {
        g_logPath = std::filesystem::path(modulePath).parent_path() / "eni.log";
    } else {
        g_logPath = std::filesystem::path("eni.log");
    }

    g_logFile.open(g_logPath, std::ios::out | std::ios::app);
    g_initialized = g_logFile.is_open();
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(g_logMutex);
    if (g_logFile.is_open()) {
        g_logFile.flush();
        g_logFile.close();
    }
    g_initialized = false;
}

void Logger::Log(const char* text, LogType logType) {
    Logf(logType, "%s", text ? text : "");
}

void Logger::Logf(LogType logType, const char* format, ...) {
    if (!g_initialized) {
        Init();
    }

    char message[2048]{};

    va_list args;
    va_start(args, format);
    vsnprintf_s(message, sizeof(message), _TRUNCATE, format ? format : "", args);
    va_end(args);

    std::string line;
    line.reserve(32 + std::strlen(message));
    line += GetTimestamp();
    line += " ";
    line += GetPrefix(logType);
    line += " ";
    line += message;

    WriteLine(line);
}

void Logger::Watermark() {
    Log("Watermark", LogType::None);
}

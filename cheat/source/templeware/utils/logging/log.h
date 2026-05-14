#pragma once
#include <string>

enum LogType {
	Info,
	Warning,
	Error,
	None
};

class Logger {
public:
	static void Init();
	static void Shutdown();

	static void Log(const char* text, LogType = LogType::Info);
	static void Logf(LogType logType, const char* format, ...);
	static void Watermark();
};
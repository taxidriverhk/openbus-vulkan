#pragma once

#include <string>

enum class LogLevel
{
    Fatal,
    Error,
    Warning,
    Info,
    Debug
};

class Logger
{
public:
    static void Log(const LogLevel level, const char *message);
    static void Log(const LogLevel level, std::string message);

private:
    static char * LOG_FILE_NAME;
    static int MAX_LOG_FILE_SIZE;
    static int MAX_LOG_FILE_COUNT;
    static bool isInitialized;

    static void Initialize();
};

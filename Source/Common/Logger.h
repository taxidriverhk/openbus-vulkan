#pragma once

#include <list>
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
    static constexpr char *LOG_FILE_NAME = "application.log";

    static std::wstring GetJoinedMessage();
    static bool IsUpdated();

    static void Log(const LogLevel level, const char *message, ...);
    static void Log(const LogLevel level, std::string message, ...);

private:
    static constexpr int MAX_LOG_FILE_SIZE = 1000000;
    static constexpr int MAX_LOG_FILE_COUNT = 5;

    static bool isInitialized;

    static void Initialize();
};

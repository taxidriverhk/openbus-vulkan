#pragma once

#include <list>
#include <string>

#include <fmt/core.h>
#include <plog/Log.h>

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

    template<typename ... Args>
    static void Log(const LogLevel level, std::string format, Args... args)
    {
        if (!isInitialized)
        {
            Initialize();
            isInitialized = true;
        }

        std::string formatted = fmt::format(format, args...);
        if (level == LogLevel::Fatal)
        {
            PLOG_FATAL << formatted;
        }
        else if (level == LogLevel::Error)
        {
            PLOG_ERROR << formatted;
        }
        else if (level == LogLevel::Warning)
        {
            PLOG_WARNING << formatted;
        }
        else if (level == LogLevel::Info)
        {
            PLOG_INFO << formatted;
        }
        else if (level == LogLevel::Debug)
        {
            PLOG_DEBUG << formatted;
        }
    }

private:
    static constexpr int MAX_LOG_FILE_SIZE = 1000000;
    static constexpr int MAX_LOG_FILE_COUNT = 5;

    static bool isInitialized;

    static void Initialize();
};

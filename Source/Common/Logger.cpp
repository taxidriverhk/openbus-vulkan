#include <cstring>

#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Initializers/RollingFileInitializer.h>

#include "Logger.h"

bool Logger::isInitialized = false;
char * Logger::LOG_FILE_NAME = "application.log";
int Logger::MAX_LOG_FILE_SIZE = 1000000;
int Logger::MAX_LOG_FILE_COUNT = 5;

void Logger::Log(const LogLevel level, const char *format, ...)
{
    if (!isInitialized)
    {
        Initialize();
        isInitialized = true;
    }

    va_list args;
    va_start(args, format);

    size_t bufferSize = snprintf(NULL, 0, format, args);
    char *buffer = new char[bufferSize + 1];

    vsprintf(buffer, format, args);
    std::string formatted(buffer);

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

    delete[] buffer;
    va_end(args);
}

void Logger::Log(const LogLevel level, std::string message, ...)
{
    const char *format = message.c_str();
    va_list args;
    va_start(args, format);

    Log(level, format, args);

    va_end(args);
}

void Logger::Initialize()
{
    static plog::RollingFileAppender<plog::TxtFormatterUtcTime> fileAppender(LOG_FILE_NAME, MAX_LOG_FILE_SIZE, MAX_LOG_FILE_COUNT);
    static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::debug, &fileAppender).addAppender(&consoleAppender);
}

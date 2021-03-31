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

void Logger::Log(const LogLevel level, const char *message)
{
    if (!isInitialized)
    {
        Initialize();
        isInitialized = true;
    }

    if (level == LogLevel::Fatal)
    {
        PLOG_FATAL << message;
    }
    else if (level == LogLevel::Error)
    {
        PLOG_ERROR << message;
    }
    else if (level == LogLevel::Warning)
    {
        PLOG_WARNING << message;
    }
    else if (level == LogLevel::Info)
    {
        PLOG_INFO << message;
    }
    else if (level == LogLevel::Debug)
    {
        PLOG_DEBUG << message;
    }
}

void Logger::Log(const LogLevel level, std::string message)
{
    Log(level, message.c_str());
}

void Logger::Initialize()
{
    static plog::RollingFileAppender<plog::TxtFormatterUtcTime> fileAppender(LOG_FILE_NAME, MAX_LOG_FILE_SIZE, MAX_LOG_FILE_COUNT);
    static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::debug, &fileAppender).addAppender(&consoleAppender);
}

#include <cstring>

#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Initializers/RollingFileInitializer.h>

#include "Logger.h"

namespace plog
{
    template<class Formatter>
    class InMemoryAppender : public IAppender
    {
    public:
        InMemoryAppender()
            : updated(false)
        {
        }

        virtual void write(const Record &record)
        {
            updated = true;
            util::nstring formatted = Formatter::format(record);
            message.append(formatted);
        }

        std::wstring GetMessage()
        {
            updated = false;
            std::wstring result = message;
            message.clear();
            return result;
        }

        bool IsUpdated() const { return updated; }

    private:
        bool updated;
        std::wstring message;
    };
}

static plog::InMemoryAppender<plog::TxtFormatterUtcTime> inMemoryAppender;

bool Logger::isInitialized = false;

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

    vsprintf_s(buffer, bufferSize + 1, format, args);
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

std::wstring Logger::GetJoinedMessage()
{
    return inMemoryAppender.GetMessage();
}

bool Logger::IsUpdated()
{
    return inMemoryAppender.IsUpdated();
}

void Logger::Initialize()
{
    static plog::RollingFileAppender<plog::TxtFormatterUtcTime> fileAppender(LOG_FILE_NAME, MAX_LOG_FILE_SIZE, MAX_LOG_FILE_COUNT);
    plog::init(plog::debug, &fileAppender).addAppender(&inMemoryAppender);
}

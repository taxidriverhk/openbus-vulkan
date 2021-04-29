#include <cstring>

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

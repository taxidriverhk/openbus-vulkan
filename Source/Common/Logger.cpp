#include <cstring>
#include <unordered_map>

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
            : enableHtml(true),
              updated(false)
        {
            severityColorMap.insert(
                {
                    { Severity::debug, L"purple" },
                    { Severity::error, L"red" },
                    { Severity::fatal, L"red" },
                    { Severity::info, L"green" },
                    { Severity::none, L"black" },
                    { Severity::verbose, L"purple" },
                    { Severity::warning, L"orange" }
                });
        }

        virtual void write(const Record &record)
        {
            updated = true;
            util::nstring formatted = Formatter::format(record);
            if (enableHtml)
            {
                std::wstring htmlFormatted = L"<span style=\"color:"
                    + severityColorMap[record.getSeverity()]
                    + L";\">"
                    + formatted
                    + L"</span><br />";
                message.append(htmlFormatted);
            }
            else
            {
                message.append(formatted);
            }
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
        bool enableHtml;
        std::unordered_map<Severity, std::wstring> severityColorMap;

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

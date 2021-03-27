#include "Util.h"

namespace Util
{
    std::string FormatWindowTitle(const std::string& subTitle)
    {
        return fmt::format("{} | {} | {}",
            APP_NAME, APP_VERSION, subTitle);
    }
}

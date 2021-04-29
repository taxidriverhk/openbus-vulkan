#include <fmt/core.h>

#include "Util.h"

std::string Util::FormatWindowTitle(const std::string &subTitle)
{
    return fmt::format("{} | {} | {}",
        APP_NAME, APP_VERSION, subTitle);
}

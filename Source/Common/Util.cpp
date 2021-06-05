#include <fmt/core.h>

#include "Util.h"

std::string Util::FormatWindowTitle(const std::string &subTitle)
{
    return fmt::format("{} | {} | {}",
        APP_NAME, APP_VERSION, subTitle);
}

std::string Util::Format3DPoint(float x, float y, float z)
{
    return fmt::format("({:.2f}, {:.2f}, {:.2f})", x, y, z);
}


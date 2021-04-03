#pragma once

#include <fmt/core.h>
#include <chrono>
#include <string>

#include "Constants.h"

class Util
{
public:
    static std::string FormatWindowTitle(const std::string &subTitle);
    static float DeltaTime();

private:
    static std::chrono::steady_clock::time_point lastTime;
};

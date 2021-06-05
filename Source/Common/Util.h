#pragma once

#include <string>

#include "Constants.h"

class Util
{
public:
    static std::string FormatWindowTitle(const std::string &subTitle);
    static std::string Format3DPoint(float x, float y, float z);
};

#pragma once

#include <string>

class Identifier
{
public:
    static uint32_t GenerateIdentifier(int x, int y)
    {
        return (x << 16) ^ y;
    }

    static uint32_t GenerateIdentifier(std::string str)
    {
        return static_cast<uint32_t>(std::hash<std::string>()(str));
    }
};

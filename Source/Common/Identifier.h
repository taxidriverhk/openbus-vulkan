#pragma once

#include <string>

// Offsets to avoid collision
enum class IdentifierType : uint32_t
{
    MapBlock = 100,
    Entity = 50000,
    AnimatedEntity = 250000,
    ScreenObject = 500000
};

class Identifier
{
public:
    static uint32_t GenerateIdentifier(IdentifierType type, int id)
    {
        return static_cast<uint32_t>(type) + id;
    }

    static uint32_t GenerateIdentifier(IdentifierType type, int x, int y)
    {
        return static_cast<uint32_t>(type) + ((x << 16) ^ y);
    }

    static uint32_t GenerateIdentifier(std::string str)
    {
        return static_cast<uint32_t>(std::hash<std::string>()(str));
    }
};

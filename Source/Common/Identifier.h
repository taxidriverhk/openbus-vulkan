#pragma once

#include <string>

// Offsets to avoid collision
enum class IdentifierType : uint32_t
{
    MapBlock = 100,
    Entity = 50000,
    GameObjectEntity = 10000000,
    ScreenObject = 20000000,
    DebugInfo = 30000000,
    RoadObject = 40000000
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

    static uint32_t GenerateIdentifier(IdentifierType type, int x, int y, int z)
    {
        return static_cast<uint32_t>(type) + ((x << 16) ^ (y << 8) ^ z);
    }

    static uint32_t GenerateIdentifier(std::string str)
    {
        return static_cast<uint32_t>(std::hash<std::string>()(str));
    }
};

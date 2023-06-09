#pragma once

#include <string>

struct Vector2DConfig
{
    float x;
    float y;
};

struct Vector3DConfig
{
    float x;
    float y;
    float z;
};

struct EntityConfig
{
    uint32_t id;
    Vector3DConfig position;
    Vector3DConfig rotation;
    std::string object;
};

struct RoadInfoConfig
{
    Vector3DConfig position;
    float rotationZ;
    float radius;
    float length;
    std::string road;
};

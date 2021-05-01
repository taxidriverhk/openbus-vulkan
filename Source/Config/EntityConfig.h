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
    Vector3DConfig position;
    Vector3DConfig rotation;
    std::string objectFilePath;
};

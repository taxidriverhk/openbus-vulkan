#pragma once

#include <string>

#include "EntityConfig.h"

struct MaterialConfig
{
    std::string diffuse;
    std::string specular;
    std::string alpha;
};

struct RoadMeshInfo
{
    Vector2DConfig startPosition;
    Vector2DConfig startTextureCoord;
    Vector2DConfig endPosition;
    Vector2DConfig endTextureCoord;
    MaterialConfig material;
};

struct RoadObjectConfig
{
    std::string name;
    std::vector<RoadMeshInfo> meshes;
};

struct StaticObjectConfig
{
    std::string name;
    std::string mesh;
    MaterialConfig material;
};

#pragma once

#include <string>

#include "EntityConfig.h"
#include "TerrainConfig.h"

struct MapBlockFileConfig
{
    Vector2DConfig position;
    std::string file;
};

struct MapInfoConfig
{
    std::string name;
    std::string image;
    std::vector<MapBlockFileConfig> blocks;
};

struct MapBlockInfoConfig
{
    Vector2DConfig offset;
    TerrainConfig terrain;
    std::vector<EntityConfig> entities;
};

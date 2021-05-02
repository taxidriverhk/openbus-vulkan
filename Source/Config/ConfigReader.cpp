#include "Common/JsonParser.h"
#include "ConfigReader.h"

#include "EntityConfig.h"
#include "MapConfig.h"
#include "ObjectConfig.h"
#include "TerrainConfig.h"

bool ConfigReader::registered = false;

void ConfigReader::RegisterConfigMapping()
{
    JsonParser::RegisterMapper(&Vector2DConfig::x, "x");
    JsonParser::RegisterMapper(&Vector2DConfig::y, "y");
    JsonParser::RegisterMapper(&Vector3DConfig::x, "x");
    JsonParser::RegisterMapper(&Vector3DConfig::y, "y");
    JsonParser::RegisterMapper(&Vector3DConfig::z, "z");

    JsonParser::RegisterMapper(&StaticObjectConfig::name, "name");
    JsonParser::RegisterMapper(&StaticObjectConfig::mesh, "mesh");
    JsonParser::RegisterMapper(&StaticObjectConfig::diffuseMaterial, "diffuseMaterial");

    JsonParser::RegisterMapper(&EntityConfig::id, "id");
    JsonParser::RegisterMapper(&EntityConfig::position, "position");
    JsonParser::RegisterMapper(&EntityConfig::rotation, "rotation");
    JsonParser::RegisterMapper(&EntityConfig::object, "object");

    JsonParser::RegisterMapper(&TerrainConfig::heightMap, "heightMap");
    JsonParser::RegisterMapper(&TerrainConfig::baseTexture, "baseTexture");
    JsonParser::RegisterMapper(&TerrainConfig::textureSize, "textureSize");

    JsonParser::RegisterMapper(&MapBlockInfoConfig::offset, "offset");
    JsonParser::RegisterMapper(&MapBlockInfoConfig::terrain, "terrain");
    JsonParser::RegisterMapper(&MapBlockInfoConfig::entities, "entities");

    JsonParser::RegisterMapper(&MapBlockFileConfig::position, "position");
    JsonParser::RegisterMapper(&MapBlockFileConfig::file, "file");

    JsonParser::RegisterMapper(&MapInfoConfig::name, "name");
    JsonParser::RegisterMapper(&MapInfoConfig::image, "image");
    JsonParser::RegisterMapper(&MapInfoConfig::blocks, "blocks");
}

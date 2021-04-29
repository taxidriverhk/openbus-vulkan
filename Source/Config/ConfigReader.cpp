#include "Common/JsonParser.h"
#include "ConfigReader.h"

#include "MapConfig.h"

bool ConfigReader::registered = false;

void ConfigReader::RegisterConfigMapping()
{
    JsonParser::RegisterMapper(&MapInfoConfig::name, "name");
    JsonParser::RegisterMapper(&MapInfoConfig::image, "image");
    JsonParser::RegisterMapper(&MapInfoConfig::blocks, "blocks");
}

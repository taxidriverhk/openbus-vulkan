#include <unordered_set>
#include <glm/glm.hpp>

#include "Common/FileSystem.h"
#include "Common/Logger.h"
#include "Config/ConfigReader.h"
#include "Config/MapConfig.h"
#include "Map.h"

Map::Map(const std::string &configFile)
    : configFilePath(configFile),
      currentBlock(nullptr),
      previousBlock(nullptr)
{
    ConfigReader::ReadConfig(configFile, mapInfoConfig);
}

Map::~Map()
{
}

void Map::AddLoadedBlock(const MapBlock &mapBlock)
{
    loadedBlocks.insert(std::make_pair(mapBlock.position, mapBlock));
}

bool Map::GetMapBlockFile(const MapBlockPosition &mapBlockPosition, MapBlockFileConfig &mapBlockFile)
{
    for (const auto &fileInfo : mapInfoConfig.blocks)
    {
        if (fileInfo.position.x == mapBlockPosition.x
            && fileInfo.position.y == mapBlockPosition.y)
        {
            mapBlockFile = fileInfo;
            return true;
        }
    }
    return false;
}

std::string Map::GetSkyBoxImageFilePath() const
{
    std::string mapBaseDirectory = FileSystem::GetParentDirectory(configFilePath);
    std::string textureFilePath = FileSystem::GetTextureFile(mapBaseDirectory, mapInfoConfig.skyBoxImage);
    return textureFilePath;
}

bool Map::IsBlockLoaded(const MapBlockPosition &mapBlockPosition)
{
    return loadedBlocks.count(mapBlockPosition) > 0;
}

void Map::Load()
{
}

bool Map::UpdateBlockPosition(const glm::vec3 &cameraPosition)
{
    int blockPositionX = static_cast<int>(cameraPosition.x) / MAP_BLOCK_SIZE,
        blockPositionY = static_cast<int>(cameraPosition.y) / MAP_BLOCK_SIZE;
    if (cameraPosition.x < 0.0f)
    {
        blockPositionX -= 1;
    }
    if (cameraPosition.y < 0.0f)
    {
        blockPositionY -= 1;
    }

    MapBlockPosition currentBlockPosition{ blockPositionX, blockPositionY };
    // Update the current block, if blocks are different from the previous one
    // Then this would trigger blocks addition/removal
    previousBlock = currentBlock;
    if (loadedBlocks.count(currentBlockPosition) > 0)
    {
        currentBlock = &loadedBlocks[currentBlockPosition];
    }

    return previousBlock != nullptr
        && currentBlock != nullptr
        && previousBlock->position != currentBlock->position;
}

std::list<uint32_t> Map::UnloadBlocks(const std::unordered_set<MapBlockPosition> &mapBlocksToKeep)
{
    std::list<uint32_t> mapBlockIdsToUnload;
    std::list<MapBlockPosition> mapBlockPositionsToUnload;
    for (const auto &[loadedPosition, loadedBlock] : loadedBlocks)
    {
        if (mapBlocksToKeep.count(loadedPosition) == 0)
        {
            mapBlockIdsToUnload.push_back(loadedBlock.id);
            mapBlockPositionsToUnload.push_back(loadedPosition);
        }
    }

    for (const auto &mapBlockPositionToUnload : mapBlockPositionsToUnload)
    {
        loadedBlocks.erase(mapBlockPositionToUnload);
    }

    return mapBlockIdsToUnload;
}

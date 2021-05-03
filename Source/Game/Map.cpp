#include <optional>
#include <unordered_map>

#include "Common/FileSystem.h"
#include "Common/HandledThread.h"
#include "Common/Identifier.h"
#include "Common/Logger.h"
#include "Config/ConfigReader.h"
#include "Config/MapConfig.h"
#include "Config/ObjectConfig.h"
#include "Engine/Image.h"
#include "Engine/Material.h"
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

MapLoader::MapLoader(Map *map, const MapLoadSettings &mapLoadSettings)
    : meshLoader(),
      terrainLoader(MAP_BLOCK_SIZE, 10, 50),
      loadProgress(0),
      readyToBuffer(false),
      shouldTerminate(false),
      firstBlockLoaded(false),
      map(map),
      mapLoadSettings(mapLoadSettings)
{
}

MapLoader::~MapLoader()
{
    if (loadBlockThread != nullptr)
    {
        loadBlockThread->Join();
    }
}

void MapLoader::AddBlocksToLoad()
{
    int maxAdjacentBlocks = mapLoadSettings.maxAdjacentBlocks;
    // Treat it as in origin if no block has been loaded yet
    // (which should only happen in the very beginning)
    MapBlock *currentBlock = map->GetCurrentBlock();
    MapBlock *previousBlock = map->GetPreviousBlock();
    MapBlockPosition currentBlockPosition = currentBlock != nullptr
        ? currentBlock->position
        : MapBlockPosition{ 0, 0 };
    // Perform add block action only if the current block position
    // is changed
    if (currentBlock != previousBlock || !firstBlockLoaded)
    {
        // Add all adjacent blocks to load regardless of block loaded/exists
        // or not, the load block load will be able to determine
        std::unordered_set<MapBlockPosition> positionsToAdd = GetAdjacentBlocks(currentBlockPosition);
        for (const MapBlockPosition &positionToAdd : positionsToAdd)
        {
            AddBlockToLoad(positionToAdd);
        }
        firstBlockLoaded = true;
    }
}

void MapLoader::AddBlockToLoad(const MapBlockPosition &mapBlockPosition)
{
    loadQueueMutex.lock();
    mapBlockLoadQueue.push(mapBlockPosition);
    loadQueueMutex.unlock();
}

std::unordered_set<MapBlockPosition> MapLoader::GetAdjacentBlocks()
{
    return GetAdjacentBlocks(map->GetCurrentBlock()->position);
}

std::unordered_set<MapBlockPosition> MapLoader::GetAdjacentBlocks(const MapBlockPosition &mapBlockPosition)
{
    std::unordered_set<MapBlockPosition> positions;
    int maxAdjacentBlocks = mapLoadSettings.maxAdjacentBlocks;
    for (int i = -maxAdjacentBlocks; i <= maxAdjacentBlocks; i++)
    {
        for (int j = -maxAdjacentBlocks; j <= maxAdjacentBlocks; j++)
        {
            positions.insert({ mapBlockPosition.x + i, mapBlockPosition.y + j });
        }
    }
    return positions;
}

MapBlockResources MapLoader::PollLoadedResources()
{
    MapBlockResources result{};
    if (readyToBuffer)
    {
        result = loadedResources.front();
        loadedResources.pop_front();
        readyToBuffer = false;
    }
    return result;
}

void MapLoader::StartLoadBlocksThread()
{
    loadBlockThread = std::make_unique<HandledThread>([&]()
        {
            while (!shouldTerminate)
            {
                // Don't push any resource to the list if the rendering thread is reading
                if (readyToBuffer)
                {
                    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME_SECONDS));
                    continue;
                }

                // Poll one map block load request from the queue at a time, and load the resources
                std::optional<MapBlockPosition> mapBlockPosition = {};
                loadQueueMutex.lock();
                if (!mapBlockLoadQueue.empty())
                {
                    mapBlockPosition = mapBlockLoadQueue.front();
                    mapBlockLoadQueue.pop();
                }
                loadQueueMutex.unlock();

                if (mapBlockPosition.has_value())
                {
                    MapBlockPosition mapBlockPositionValue = mapBlockPosition.value();
                    if (map->IsBlockLoaded(mapBlockPositionValue))
                    {
                        continue;
                    }

                    // Attempt to load the map block file
                    // Skip if no matching file is found
                    MapBlockFileConfig mapBlockFileConfig;
                    if (!map->GetMapBlockFile(mapBlockPositionValue, mapBlockFileConfig))
                    {
                        continue;
                    }

                    Logger::Log(LogLevel::Info, "Loading resources for block ({}, {})",
                        mapBlockPositionValue.x, mapBlockPositionValue.y);
                    std::string mapBaseDirectory = FileSystem::GetParentDirectory(map->GetConfigFilePath());
                    // Load the list of entity files from the map block config file
                    std::string mapBlockFilePath = FileSystem::GetMapBlockFile(mapBaseDirectory, mapBlockFileConfig.file);
                    MapBlockInfoConfig mapBlockInfoConfig;
                    if (!ConfigReader::ReadConfig(mapBlockFilePath, mapBlockInfoConfig))
                    {
                        Logger::Log(LogLevel::Warning, "Failed to load map block file config from {}", mapBlockFilePath);
                        continue;
                    }

                    float mapBlockOffsetX = static_cast<float>(mapBlockPositionValue.x) * MAP_BLOCK_SIZE,
                          mapBlockOffsetY = static_cast<float>(mapBlockPositionValue.y) * MAP_BLOCK_SIZE;
                    uint32_t blockId = Identifier::GenerateIdentifier(mapBlockPositionValue.x, mapBlockPositionValue.y);

                    MapBlockResources mapBlockResource;
                    mapBlockResource.blockId = blockId;

                    // Load the terrain from height map with texture
                    Terrain &terrain = mapBlockResource.terrain;
                    terrain.id = blockId;
                    const TerrainConfig &terrainConfig = mapBlockInfoConfig.terrain;
                    std::string heightMapPath = FileSystem::GetHeightMapFile(mapBaseDirectory, terrainConfig.heightMap);
                    std::string textureFilePath = FileSystem::GetTextureFile(mapBaseDirectory, terrainConfig.baseTexture);
                    if (!terrainLoader.LoadFromHeightMap(
                        heightMapPath,
                        glm::vec3(mapBlockOffsetX, mapBlockOffsetY, 0.0f),
                        terrainConfig.textureSize,
                        textureFilePath,
                        terrain))
                    {
                        Logger::Log(LogLevel::Warning, "Failed to load terrain for block ({}, {})",
                            mapBlockInfoConfig.offset.x, mapBlockInfoConfig.offset.y);
                        continue;
                    }

                    // Load the entities grouped by object file to avoid loading the same object more than once
                    std::vector<Entity> &entities = mapBlockResource.entities;
                    std::vector<EntityConfig> &entityConfigs = mapBlockInfoConfig.entities;

                    std::unordered_map<uint32_t, std::shared_ptr<Mesh>> objectIdMeshMap;
                    std::unordered_map<uint32_t, std::shared_ptr<Material>> materialIdImageMap;
                    for (const EntityConfig &entityConfig : entityConfigs)
                    {
                        Entity entity;

                        std::string objectFile = entityConfig.object;
                        uint32_t objectId = Identifier::GenerateIdentifier(objectFile);
                        if (objectIdMeshMap.count(objectId) == 0)
                        {
                            Mesh mesh;
                            mesh.id = objectId;

                            std::string objectFilePath = FileSystem::GetObjectFile(objectFile);
                            std::string objectBaseDirectory = FileSystem::GetParentDirectory(objectFilePath);

                            StaticObjectConfig staticObjectConfig;
                            if (!ConfigReader::ReadConfig(objectFilePath, staticObjectConfig))
                            {
                                Logger::Log(LogLevel::Warning, "Failed to load object config from file {}", objectFilePath);
                                continue;
                            }

                            std::string modelFilePath = FileSystem::GetModelFile(objectBaseDirectory, staticObjectConfig.mesh);
                            if (!meshLoader.LoadFromFile(modelFilePath, mesh))
                            {
                                Logger::Log(LogLevel::Warning, "Failed to load mesh from object {}", objectFile);
                                continue;
                            }

                            std::string textureFilePath = FileSystem::GetTextureFile(objectBaseDirectory, staticObjectConfig.diffuseMaterial);
                            uint32_t materialId = Identifier::GenerateIdentifier(textureFilePath);
                            if (materialIdImageMap.count(materialId) == 0)
                            {
                                Material material;
                                material.id = materialId;
                                
                                std::shared_ptr<Image> diffuseImage = std::make_shared<Image>();
                                if (!diffuseImage->Load(textureFilePath, ImageColor::ColorWithAlpha))
                                {
                                    Logger::Log(LogLevel::Warning, "Failed to load texture from file {}", textureFilePath);
                                    continue;
                                }

                                material.diffuseImage = diffuseImage;
                                materialIdImageMap[materialId] = std::make_shared<Material>(material);
                            }

                            mesh.material = materialIdImageMap[materialId];
                            objectIdMeshMap[objectId] = std::make_shared<Mesh>(mesh);
                        }

                        entity.id = entityConfig.id;
                        entity.translation =
                        { 
                            mapBlockOffsetX + entityConfig.position.x,
                            mapBlockOffsetY + entityConfig.position.y,
                            entityConfig.position.z
                        };
                        entity.rotation =
                        {
                            entityConfig.rotation.x,
                            entityConfig.rotation.y,
                            entityConfig.rotation.z
                        };
                        entity.scale = { 1.0f, 1.0f, 1.0f };
                        entity.mesh = objectIdMeshMap[objectId];
                        entities.push_back(entity);
                    }

                    MapBlock loadedMapBlock{};
                    loadedMapBlock.id = blockId;
                    loadedMapBlock.position = mapBlockPositionValue;
                    map->AddLoadedBlock(loadedMapBlock);
                    loadedResources.push_back(mapBlockResource);
                    readyToBuffer = true;
                }
            }
        },
        [&]()
        {
            TerminateLoadBlocksThread();
        });
}

void MapLoader::TerminateLoadBlocksThread()
{
    shouldTerminate = true;
}

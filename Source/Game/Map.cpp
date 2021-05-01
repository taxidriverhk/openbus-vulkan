#include <optional>

#include "Common/FileSystem.h"
#include "Common/HandledThread.h"
#include "Config/ConfigReader.h"
#include "Config/MapConfig.h"
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
    for (const auto &fileInfo : mapInfoConfig.blockFiles)
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
    return false;
}

void Map::Load()
{
}

void Map::UpdateBlockPosition(const glm::vec3 &cameraPosition)
{
    // TODO: determine the current block position based on the camera position
    MapBlockPosition currentBlockPosition{ 0, 0 };
    // Update the current block, if blocks are different from each other
    // Then this would trigger blocks addition/removal
    previousBlock = currentBlock;
    if (loadedBlocks.count(currentBlockPosition) > 0)
    {
        currentBlock = &loadedBlocks[currentBlockPosition];
    }
}

MapLoader::MapLoader(Map *map, const MapLoadSettings &mapLoadSettings)
    : meshLoader(),
      terrainLoader(MAP_BLOCK_SIZE, 10, 50),
      loadProgress(0),
      readyToBuffer(false),
      shouldTerminate(false),
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
    if (currentBlock != previousBlock)
    {
        // Add all adjacent blocks to load regardless of block loaded/exists
        // or not, the load block load will be able to determine
        for (int i = -maxAdjacentBlocks; i <= maxAdjacentBlocks; i++)
        {
            for (int j = -maxAdjacentBlocks; j <= maxAdjacentBlocks; j++)
            {
                if (i == 0 && j == 0 && currentBlock != nullptr)
                {
                    continue;
                }

                MapBlockPosition positionToAdd{ currentBlockPosition.x + i, currentBlockPosition.y + j };
                AddBlockToLoad(positionToAdd);
            }
        }
    }
    // No block has been loaded, so load from origin position
    else if (currentBlock == nullptr)
    {
        AddBlockToLoad(currentBlockPosition);
    }
}

void MapLoader::AddBlockToLoad(const MapBlockPosition &mapBlockPosition)
{
    loadQueueMutex.lock();
    mapBlockLoadQueue.push(mapBlockPosition);
    loadQueueMutex.unlock();
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
                    // Attempt to load the map block file
                    // Skip if no matching file is found
                    MapBlockFileConfig mapBlockFileConfig;
                    if (!map->GetMapBlockFile(mapBlockPosition.value(), mapBlockFileConfig))
                    {
                        continue;
                    }

                    std::string mapBaseDirectory = FileSystem::GetMapBaseDirectory(map->GetConfigFilePath());
                    // Load the list of entity files from the map block config file
                    std::string mapBlockFilePath = FileSystem::GetMapBlockFile(mapBaseDirectory, mapBlockFileConfig.filePath);
                    MapBlockInfoConfig mapBlockInfoConfig;
                    if (!ConfigReader::ReadConfig(mapBlockFilePath, mapBlockInfoConfig))
                    {
                        continue;
                    }

                    float mapBlockOffsetX = mapBlockInfoConfig.offset.x * MAP_BLOCK_SIZE,
                          mapBlockOffsetY = mapBlockInfoConfig.offset.y * MAP_BLOCK_SIZE;

                    const TerrainConfig &terrainConfig = mapBlockInfoConfig.terrain;
                    std::string heightMapPath = FileSystem::GetHeightMapFile(mapBaseDirectory, terrainConfig.heightMapFilePath);
                    std::string textureFilePath = FileSystem::GetTextureFile(mapBaseDirectory, terrainConfig.baseTextureFilePath);

                    // TODO: hard-coded the things to load for now
                    MapBlockResources mapBlockResource;
                    Terrain &terrain = mapBlockResource.terrain;
                    std::vector<Entity> &entities = mapBlockResource.entities;

                    terrain.id = 555;
                    if (!terrainLoader.LoadFromHeightMap(
                        heightMapPath,
                        glm::vec3(mapBlockOffsetX, mapBlockOffsetY, 0.0f),
                        terrainConfig.textureSize,
                        textureFilePath,
                        terrain))
                    {
                        continue;
                    }

                    uint32_t numberOfMeshes = 10;
                    Material materials[] =
                    {
                        {
                            1,
                            std::make_shared<Image>("formula1.png"),
                            nullptr,
                            nullptr
                        },
                        {
                            4,
                            std::make_shared<Image>("wallpaper.bmp"),
                            nullptr,
                            nullptr
                        }
                    };

                    Mesh formula1{}, wallpaper;
                    if (!meshLoader.LoadFromFile("formula1.obj", formula1)
                        || !meshLoader.LoadFromFile("wallpaper.obj", wallpaper))
                    {
                        continue;
                    }

                    for (uint32_t index = 0; index < numberOfMeshes; index++)
                    {
                        formula1.id = 1;
                        formula1.material = std::make_shared<Material>(materials[0]);

                        wallpaper.id = 2;
                        wallpaper.material = std::make_shared<Material>(materials[1]);

                        Entity entity1{};
                        entity1.id = index;
                        entity1.mesh = std::make_shared<Mesh>(formula1);
                        entity1.translation = { index * 25.0f , 5.0f, 0.5f };
                        entity1.scale = { 1.0f, 1.0f, 1.0f };

                        Entity entity2{};
                        entity2.id = index * 2;
                        entity2.mesh = std::make_shared<Mesh>(wallpaper);
                        entity2.translation = { index * 2.0f , 0.0f, 0.0f };
                        entity2.scale = { 1.0f, 1.0f, 1.0f };

                        entities.push_back(entity1);
                        entities.push_back(entity2);
                    }

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

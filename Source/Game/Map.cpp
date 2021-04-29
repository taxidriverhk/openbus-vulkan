#include <optional>

#include "Config/ConfigReader.h"
#include "Config/MapConfig.h"
#include "Engine/Image.h"
#include "Engine/Material.h"
#include "Map.h"

Map::Map()
    : currentBlock(nullptr)
{
}

Map::~Map()
{
}

void Map::Load(const std::string &configFile)
{
    MapInfoConfig mapConfig;
    ConfigReader::ReadConfig(configFile, mapConfig);
}

MapLoader::MapLoader(const MapInfoConfig &mapConfig)
    : meshLoader(),
      terrainLoader(1000, 10, 50, 50),
      loadProgress(0),
      readyToBuffer(false),
      shouldTerminate(false),
      mapConfig(mapConfig)
{
}

MapLoader::~MapLoader()
{
    if (loadBlockThread != nullptr)
    {
        loadBlockThread->join();
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
    loadBlockThread = std::make_unique<std::thread>([&]()
        {
            while (!shouldTerminate)
            {
                // Don't push any resource to the list if the rendering thread is reading
                if (readyToBuffer)
                {
                    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME_SECONDS));
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
                    // TODO: hard-coded the things to load for now
                    MapBlockResources mapBlockResource;
                    Terrain &terrain = mapBlockResource.terrain;
                    std::vector<Entity> &entities = mapBlockResource.entities;

                    terrain.id = 555;
                    terrainLoader.LoadFromHeightMap("heightmap.png", glm::vec3(0.0f, 0.0f, 0.0f), terrain);
                    terrain.texture = std::make_shared<Image>("grass.bmp");

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
                        throw std::runtime_error("Failed to load models from files");
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
        });
}

void MapLoader::TerminateLoadBlocksThread()
{
    shouldTerminate = true;
}

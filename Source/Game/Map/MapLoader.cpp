#include <algorithm>
#include <execution>
#include <optional>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Common/FileSystem.h"
#include "Common/HandledThread.h"
#include "Common/Identifier.h"
#include "Common/Logger.h"
#include "Config/ConfigReader.h"
#include "Config/ObjectConfig.h"
#include "Engine/Image.h"
#include "Engine/Material.h"
#include "MapLoader.h"

MapLoader::MapLoader(Map *map, const MapLoadSettings &mapLoadSettings)
    : meshLoader(),
    terrainLoader(MAP_BLOCK_SIZE, 10, 50),
    staticEntityIdCount(0),
    loadProgress(0),
    readyToBuffer(false),
    readyToAdd(false),
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

MapBlockSurfaces MapLoader::PollLoadedSurfaces()
{
    MapBlockSurfaces result{};
    if (readyToAdd)
    {
        result = loadedSurfaces.front();
        loadedSurfaces.pop_front();
        readyToAdd = false;
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
                    uint32_t blockId = Identifier::GenerateIdentifier(IdentifierType::MapBlock, mapBlockPositionValue.x, mapBlockPositionValue.y);

                    MapBlock loadedMapBlock{};
                    loadedMapBlock.id = blockId;
                    loadedMapBlock.position = mapBlockPositionValue;

                    MapBlockResources mapBlockResource;
                    mapBlockResource.blockId = blockId;

                    MapBlockSurfaces mapBlockSurface;
                    mapBlockSurface.blockId = blockId;

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

                    CollisionMesh terrainCollisionMesh;
                    terrainCollisionMesh.vertices.resize(terrain.vertices.size());
                    terrainCollisionMesh.indices.resize(terrain.indices.size());
                    std::transform(
                        std::execution::par,
                        terrain.vertices.begin(),
                        terrain.vertices.end(),
                        terrainCollisionMesh.vertices.begin(),
                        [&](Vertex &vertex)
                        {
                            return glm::vec3{ vertex.position.x, -vertex.position.y, vertex.position.z };
                        });
                    std::copy(terrain.indices.begin(), terrain.indices.end(), terrainCollisionMesh.indices.begin());
                    mapBlockSurface.collisionMeshes.push_back(terrainCollisionMesh);

                    loadedMapBlock.terrainMapItem.id = terrain.id;

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

                            std::string objectFilePath = FileSystem::GetStaticObjectFile(objectFile);
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

                            std::string textureFilePath = FileSystem::GetTextureFile(objectBaseDirectory, staticObjectConfig.material.diffuse);
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

                        glm::vec3 entityOrigin =
                        {
                            entityConfig.position.x,
                            entityConfig.position.y,
                            entityConfig.position.z
                        };

                        entity.id = Identifier::GenerateIdentifier(IdentifierType::Entity, ++staticEntityIdCount);
                        entity.translation = entityOrigin;
                        entity.translation.x += mapBlockOffsetX;
                        entity.translation.y += mapBlockOffsetY;
                        entity.rotation =
                        {
                            entityConfig.rotation.x,
                            entityConfig.rotation.y,
                            entityConfig.rotation.z
                        };
                        entity.scale = { 1.0f, 1.0f, 1.0f };
                        entity.mesh = objectIdMeshMap[objectId];
                        entities.push_back(entity);

                        StaticObjectMapItem objectMapItem{};
                        objectMapItem.id = entity.id;
                        objectMapItem.position = entityOrigin;
                        objectMapItem.rotations = entity.rotation;
                        loadedMapBlock.staticMapItems.push_back(objectMapItem);
                    }

                    // Load the roads, where each road maps to one or more entities
                    // since each road allows multiple materials
                    std::vector<RoadInfoConfig> &roadConfigs = mapBlockInfoConfig.roads;
                    for (const RoadInfoConfig &roadConfig : roadConfigs)
                    {
                        Road road;

                        std::string roadFilePath = FileSystem::GetRoadObjectFile(roadConfig.road);
                        RoadInfo roadInfo{};
                        roadInfo.position = { roadConfig.position.x, roadConfig.position.y, roadConfig.position.z };
                        roadInfo.rotationZ = roadConfig.rotationZ;
                        roadInfo.radius = roadConfig.radius;
                        roadInfo.length = roadConfig.length;

                        if (!roadLoader.LoadFromFile(roadFilePath, roadInfo, road))
                        {
                            Logger::Log(LogLevel::Warning, "Failed to load road from file {}", roadFilePath);
                            continue;
                        }

                        RoadMapItem roadMapItem{};
                        roadMapItem.id = road.id;
                        roadMapItem.info = roadInfo;

                        for (Mesh &roadMesh : road.meshes)
                        {
                            Entity roadEntity;

                            roadEntity.id = Identifier::GenerateIdentifier(IdentifierType::Entity, ++staticEntityIdCount);
                            // Transformations are not applied within the road loader
                            roadEntity.translation = roadInfo.position;
                            roadEntity.rotation = { 0.0f, 0.0f, roadInfo.rotationZ };
                            roadEntity.scale = { 1.0f, 1.0f, 1.0f };
                            roadEntity.mesh = std::make_shared<Mesh>(roadMesh);

                            entities.push_back(roadEntity);
                            roadMapItem.entityIds.push_back(roadEntity.id);

                            // For collision mesh, we would need to apply the transformation for loading into the physics world
                            float rotationRadians = glm::radians<float>(roadInfo.rotationZ);
                            CollisionMesh roadCollisionMesh;
                            roadCollisionMesh.vertices.resize(roadMesh.vertices.size());
                            roadCollisionMesh.indices.resize(roadMesh.indices.size());
                            std::transform(
                                std::execution::par,
                                roadMesh.vertices.begin(),
                                roadMesh.vertices.end(),
                                roadCollisionMesh.vertices.begin(),
                                [&](Vertex &vertex)
                                {
                                    // Apply rotation and translations to the collision mesh of the road
                                    float cosRotation = glm::cos(-rotationRadians),
                                          sinRotation = glm::sin(-rotationRadians);
                                    float originX = vertex.position.x,
                                          originY = vertex.position.y;
                                    float rotatedX = originX * cosRotation - originY * sinRotation,
                                          rotatedY = originX * sinRotation + originY * cosRotation;
                                    glm::vec3 rotatedPosition = { rotatedX, rotatedY, vertex.position.z };
                                    return rotatedPosition + roadInfo.position;
                                });
                            std::copy(roadMesh.indices.begin(), roadMesh.indices.end(), roadCollisionMesh.indices.begin());
                            mapBlockSurface.collisionMeshes.push_back(roadCollisionMesh);
                        }

                        loadedMapBlock.roadMapItems.push_back(roadMapItem);
                    }

                    loadedResources.push_back(mapBlockResource);
                    loadedSurfaces.push_back(mapBlockSurface);
                    map->AddLoadedBlock(loadedMapBlock);

                    readyToBuffer = true;
                    readyToAdd = true;
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

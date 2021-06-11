#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <unordered_set>

#include "Config/SettingsConfig.h"
#include "Engine/Entity.h"
#include "Engine/Mesh.h"
#include "Engine/Terrain.h"
#include "Game/Physics/Collision.h"
#include "Game/Path/Road.h"
#include "Map.h"

class HandledThread;

struct MapBlockResources
{
    uint32_t blockId;
    Terrain terrain;
    std::vector<Entity> entities;

    MapBlockResources()
        : blockId(0)
    {
    }

    MapBlockResources(const MapBlockResources &other)
    {
        blockId = other.blockId;
        terrain = other.terrain;
        entities = other.entities;
    }
};

// Used to load the collision surfaces into the physics world
struct MapBlockSurfaces
{
    uint32_t blockId;
    std::vector<CollisionMesh> collisionMeshes;
};

class MapLoader
{
public:
    MapLoader(Map *map, const MapLoadSettings &mapLoadSettings);
    ~MapLoader();

    int GetProgress() const { return loadProgress; }

    bool IsReadyToBuffer() const { return readyToBuffer; }
    bool IsReadyToAdd() const { return readyToAdd; }

    void AddBlocksToLoad();
    std::unordered_set<MapBlockPosition> GetAdjacentBlocks();
    
    MapBlockResources PollLoadedResources();
    MapBlockSurfaces PollLoadedSurfaces();
    
    void StartLoadBlocksThread();
    void TerminateLoadBlocksThread();

private:
    static constexpr uint32_t WAIT_TIME_SECONDS = 1;

    void AddBlockToLoad(const MapBlockPosition &mapBlockPosition);
    std::unordered_set<MapBlockPosition> GetAdjacentBlocks(const MapBlockPosition &mapBlockPosition);

    std::atomic<uint32_t> staticEntityIdCount;

    bool firstBlockLoaded;
    Map *map;
    MapLoadSettings mapLoadSettings;

    MeshLoader meshLoader;
    RoadLoader roadLoader;
    TerrainLoader terrainLoader;

    int loadProgress;

    std::atomic<bool> readyToBuffer;
    std::list<MapBlockResources> loadedResources;

    std::atomic<bool> readyToAdd;
    std::list<MapBlockSurfaces> loadedSurfaces;

    bool shouldTerminate;
    std::mutex loadQueueMutex;
    std::unique_ptr<HandledThread> loadBlockThread;
    std::queue<MapBlockPosition> mapBlockLoadQueue;
};

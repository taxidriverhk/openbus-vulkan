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

class MapLoader
{
public:
    MapLoader(Map *map, const MapLoadSettings &mapLoadSettings);
    ~MapLoader();

    int GetProgress() const { return loadProgress; }
    bool IsReadyToBuffer() const { return readyToBuffer; }

    void AddBlocksToLoad();
    std::unordered_set<MapBlockPosition> GetAdjacentBlocks();
    MapBlockResources PollLoadedResources();
    void StartLoadBlocksThread();
    void TerminateLoadBlocksThread();

private:
    static constexpr uint32_t WAIT_TIME_SECONDS = 1;

    void AddBlockToLoad(const MapBlockPosition &mapBlockPosition);
    std::unordered_set<MapBlockPosition> GetAdjacentBlocks(const MapBlockPosition &mapBlockPosition);

    uint32_t staticEntityIdCount;

    bool firstBlockLoaded;
    Map *map;
    MapLoadSettings mapLoadSettings;

    MeshLoader meshLoader;
    TerrainLoader terrainLoader;

    int loadProgress;
    std::atomic<bool> readyToBuffer;
    std::list<MapBlockResources> loadedResources;

    bool shouldTerminate;
    std::mutex loadQueueMutex;
    std::unique_ptr<HandledThread> loadBlockThread;
    std::queue<MapBlockPosition> mapBlockLoadQueue;
};

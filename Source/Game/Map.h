#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include <unordered_set>

#include "Engine/Entity.h"
#include "Engine/Mesh.h"
#include "Engine/Terrain.h"

static constexpr int MAP_BLOCK_SIZE = 1000;

struct MapBlockPosition
{
    int x;
    int y;

    bool operator ==(const MapBlockPosition &other) const
    {
        return this->x == other.x && this->y == other.y;
    }
};

struct MapBlock
{
    // TODO: need to figure out what a map block has
    uint32_t id;
    MapBlockPosition position;
    std::vector<std::string> meshFiles;

    MapBlock operator =(const MapBlock &other)
    {
        MapBlock result{};
        result.id = other.id;
        result.position = other.position;
        result.meshFiles = std::vector<std::string>(other.meshFiles);
        return result;
    }

    bool operator ==(const MapBlock &other) const
    {
        return this->position == other.position;
    }
};

namespace std
{
    template<> struct hash<MapBlockPosition>
    {
        size_t operator()(MapBlockPosition const &mapBlockPosition) const
        {
            return (static_cast<size_t>(mapBlockPosition.x) << 8) ^ static_cast<size_t>(mapBlockPosition.y);
        }
    };

    template<> struct hash<MapBlock>
    {
        size_t operator()(MapBlock const &mapBlock) const
        {
            return hash<MapBlockPosition>()(mapBlock.position);
        }
    };
}

struct MapBlockResources
{
    Terrain terrain;
    std::vector<Entity> entities;

    MapBlockResources()
    {
    }

    MapBlockResources(const MapBlockResources &other)
    {
        terrain = other.terrain;
        entities = other.entities;
    }
};

class Map
{
public:
    Map();
    ~Map();

private:
    std::string name;
    MapBlock *currentBlock;
    std::unordered_set<MapBlock> loadedBlocks;
};

class MapLoader
{
public:
    MapLoader();
    ~MapLoader();

    int GetProgress() const { return loadProgress; }
    bool IsReadyToBuffer() const { return readyToBuffer; }

    void AddBlockToLoad(MapBlock &mapBlock);
    MapBlockResources PollLoadedResources();
    void StartLoadBlocksThread();
    void TerminateLoadBlocksThread();

private:
    static constexpr uint32_t WAIT_TIME_SECONDS = 10;

    MeshLoader meshLoader;
    TerrainLoader terrainLoader;

    int loadProgress;
    std::atomic<bool> readyToBuffer;
    std::list<MapBlockResources> loadedResources;

    bool shouldTerminate;
    std::mutex loadQueueMutex;
    std::unique_ptr<std::thread> loadBlockThread;
    std::queue<MapBlock> mapBlockLoadQueue;
};

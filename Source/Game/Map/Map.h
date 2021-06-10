#pragma once

#include <list>
#include <string>
#include <vector>
#include <unordered_map>

#include "Game/Path/Road.h"
#include "Config/MapConfig.h"

// Dimension of a map block in meters, this should never be changed
static constexpr int MAP_BLOCK_SIZE = 1000;

// Stores the transformation and the entity ID of a road
// without having to store the vertex and image data that are already loaded into the graphics
struct StaticObjectMapItem
{
    uint32_t id;
    glm::vec3 position;
    glm::vec3 rotations;
};

// Stores the transformation and other metadata of a road without
// having to store the vertex and image data that are already loaded into the graphics
struct RoadMapItem
{
    uint32_t id;
    RoadInfo info;
    std::vector<uint32_t> entityIds;
};

struct TerrainMapItem
{
    uint32_t id;
    // TODO: may store the layer information for multi-texturing
};

struct MapBlockPosition
{
    int x;
    int y;

    bool operator ==(const MapBlockPosition &other) const
    {
        return this->x == other.x && this->y == other.y;
    }

    bool operator !=(const MapBlockPosition &other) const
    {
        return this->x != other.x || this->y != other.y;
    }
};

struct MapBlock
{
    // TODO: need to figure out what a map block has
    uint32_t id;
    MapBlockPosition position;

    TerrainMapItem terrainMapItem;
    std::vector<StaticObjectMapItem> staticMapItems;
    std::vector<RoadMapItem> roadMapItems;

    MapBlock operator =(const MapBlock &other)
    {
        MapBlock result{};
        result.id = other.id;
        result.position = other.position;
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

class Map
{
public:
    Map(const std::string &configFile);
    ~Map();

    std::string GetConfigFilePath() const { return configFilePath; }
    std::string GetSkyBoxImageFilePath() const;
    MapBlock *GetCurrentBlock() const { return currentBlock; }
    MapBlock *GetPreviousBlock() const { return previousBlock; }

    void AddLoadedBlock(const MapBlock &mapBlock);
    bool GetMapBlockFile(const MapBlockPosition &mapBlockPosition, MapBlockFileConfig &mapBlockFile);
    bool IsBlockLoaded(const MapBlockPosition &mapBlockPosition);
    void Load();
    bool UpdateBlockPosition(const glm::vec3 &cameraPosition);
    std::list<uint32_t> UnloadBlocks(const std::unordered_set<MapBlockPosition> &mapBlocksToKeep);

private:
    MapBlock *previousBlock;
    MapBlock *currentBlock;
    MapInfoConfig mapInfoConfig;
    std::string configFilePath;
    std::unordered_map<MapBlockPosition, MapBlock> loadedBlocks;
};

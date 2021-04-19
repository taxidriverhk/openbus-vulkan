#pragma once

#include <string>
#include <vector>

class Image;
struct Vertex;

struct Terrain
{
    uint32_t id;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::shared_ptr<Image> texture;
};

class TerrainLoader
{
public:
    TerrainLoader(int size, int gridSize, float textureSize, float heightRange);
    ~TerrainLoader();

    bool LoadFromHeightMap(const std::string filename, Terrain &terrain);

private:
    static constexpr uint32_t MAX_PIXEL_VALUE = UCHAR_MAX * UCHAR_MAX * UCHAR_MAX;

    float CalculateHeight(const uint8_t *basePixel);

    int size;
    int gridSize;
    float textureSize;
    float heightRange;
};

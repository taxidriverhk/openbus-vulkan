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

    bool LoadFromHeightMap(
        const std::string filename,
        const glm::vec3 offset,
        Terrain &terrain);

private:
    static constexpr uint32_t MAX_PIXEL_VALUE = UCHAR_MAX * UCHAR_MAX * UCHAR_MAX;

    float CalculateHeight(const uint8_t *basePixel);
    glm::vec3 CalculateNormal(std::vector<Vertex> &vertices, uint32_t grids, uint32_t x, uint32_t y);

    int size;
    int gridSize;
    float textureSize;
    float heightRange;
};

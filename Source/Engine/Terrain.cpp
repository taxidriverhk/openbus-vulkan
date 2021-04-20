#include <math.h>

#include "Image.h"
#include "Mesh.h"
#include "Terrain.h"

TerrainLoader::TerrainLoader(int size, int gridSize, float textureSize, float heightRange)
    : size(size),
      gridSize(gridSize),
      textureSize(textureSize),
      heightRange(heightRange)
{
}

TerrainLoader::~TerrainLoader()
{
}

bool TerrainLoader::LoadFromHeightMap(
    const std::string filename,
    const glm::vec3 offset,
    Terrain &terrain)
{
    Image image;
    if (!image.Load(filename, ImageColor::ColorWithAlpha))
    {
        return false;
    }

    uint32_t grids = size / gridSize;

    float hFactor = static_cast<float>(image.GetWidth()) / grids;
    float vFactor = static_cast<float>(image.GetHeight()) / grids;
    float uvStep = gridSize / textureSize;
    uint8_t *pixels = image.GetPixels();

    std::vector<Vertex> &vertices = terrain.vertices;
    vertices.reserve(static_cast<size_t>(grids) * grids);
    for (uint32_t i = 0; i < grids; i++)
    {
        uint32_t imageRow = static_cast<uint32_t>(i * vFactor);
        float vertexPositionX = static_cast<float>(i * gridSize);
        for (uint32_t j = 0; j < grids; j++)
        {
            uint32_t imageColumn = image.GetHeight() - static_cast<uint32_t>(j * hFactor) - 1;
            uint32_t imageBaseOffset = 4 * (imageColumn * image.GetWidth() + imageRow);

            float vertexPositionY = static_cast<float>(j * gridSize);
            float height = CalculateHeight(pixels + imageBaseOffset);

            Vertex vertex{};
            vertex.position = { vertexPositionX, -vertexPositionY, height };
            vertex.position += offset;

            vertex.normal = { 0.0f, 0.0f, 1.0f };

            vertex.uv = { j * uvStep, i * uvStep };

            vertices.push_back(vertex);
        }
    }

    std::vector<uint32_t> &indices = terrain.indices;
    for (uint32_t i = 0; i < grids - 1; i++)
    {
        for (uint32_t j = 0; j < grids - 1; j++)
        {
            uint32_t base = i * grids + j;
            uint32_t nextBase = (i + 1) * grids + j;

            indices.push_back(base);
            indices.push_back(nextBase);
            indices.push_back(base + 1);
            indices.push_back(base + 1);
            indices.push_back(nextBase);
            indices.push_back(nextBase + 1);
        }
    }

    return true;
}

float TerrainLoader::CalculateHeight(const uint8_t *basePixel)
{
    uint32_t combinedPixelValue = basePixel[0]
        + UCHAR_MAX * basePixel[1]
        + UCHAR_MAX * UCHAR_MAX * basePixel[2];
    float halfMaxPixelValue = static_cast<float>(MAX_PIXEL_VALUE / 2);
    float height = ((combinedPixelValue - halfMaxPixelValue) * heightRange) / halfMaxPixelValue;
    // Round to two decimal places
    return roundf(height * 100) / 100;
}

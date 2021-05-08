#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Vertex.h"

class Image;
struct Material;

struct Mesh
{
    uint32_t id;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::shared_ptr<Material> material;
};

struct ScreenMesh
{
    uint32_t id;
    std::vector<ScreenObjectVertex> vertices;
    std::shared_ptr<Image> image;
};

class MeshLoader
{
public:
    MeshLoader();
    ~MeshLoader();

    bool LoadFromFile(const std::string filename, Mesh &mesh);

private:
};

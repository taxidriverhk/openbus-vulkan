#pragma once

#include <memory>
#include <string>
#include <vector>

struct Material;
struct Vertex;

struct Mesh
{
    uint32_t id;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::shared_ptr<Material> material;
};

class MeshLoader
{
public:
    MeshLoader();
    ~MeshLoader();

    bool LoadFromFile(const std::string filename, Mesh &mesh);

private:
};

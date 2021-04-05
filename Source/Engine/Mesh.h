#pragma once

#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Material.h"

struct Vertex
{
    glm::vec2 position;
    glm::vec3 color;
    glm::vec2 uv;
};

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
    virtual Mesh LoadFromFile(const std::string filename) = 0;
};

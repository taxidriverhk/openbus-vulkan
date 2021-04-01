#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

struct Vertex
{
    glm::vec2 position;
    glm::vec3 color;
};

struct Mesh
{
    uint32_t id;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

class MeshLoader
{
public:
    virtual Mesh LoadFromFile(const std::string filename) = 0;
};

#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Vertex.h"

class Image;
struct Mesh;

struct CubeMap
{
    std::vector<Image *> images;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

struct Entity
{
    uint32_t id;
    std::shared_ptr<Mesh> mesh;
    glm::vec3 translation;
    glm::vec3 rotation;
    glm::vec3 scale;
};

struct EntityTransformation
{
    glm::vec3 translation;
    glm::vec3 rotation;
    glm::vec3 scale;
};

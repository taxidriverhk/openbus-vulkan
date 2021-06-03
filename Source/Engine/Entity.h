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

enum class EntityTransformationMode
{
    EulerAngles,
    Quaternion,
    Matrix
};

struct EntityTransformation
{
    EntityTransformationMode mode;

    glm::vec3 translation;
    glm::vec3 scale;

    glm::vec3 rotation;

    glm::vec3 rotationAxis;
    float angle;

    glm::mat4 matrix;

    EntityTransformation()
        : mode(EntityTransformationMode::EulerAngles),
          translation{},
          scale(1.0f, 1.0f, 1.0f),
          rotation{},
          rotationAxis{},
          angle(0.0f),
          matrix(glm::identity<glm::mat4>())
    {}
};

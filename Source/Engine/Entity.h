#pragma once

class Image;
struct Mesh;

struct CubeMap
{
    std::vector<Image *> images;
};

struct Entity
{
    uint32_t id;
    std::shared_ptr<Mesh> mesh;
    glm::vec3 translation;
    glm::vec3 rotation;
    glm::vec3 scale;
};

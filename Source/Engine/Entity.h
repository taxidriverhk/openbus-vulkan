#include "Mesh.h"

struct Entity
{
    uint32_t id;
    std::shared_ptr<Mesh> mesh;
    glm::vec3 translation;
    glm::vec3 rotation;
};

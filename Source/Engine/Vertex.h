#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;

    bool operator==(const Vertex &other) const
    {
        return position == other.position
            && normal == other.normal
            && uv == other.uv;
    }
};

struct ScreenObjectVertex
{
    glm::vec3 color;
    glm::vec2 position;
    glm::vec2 uv;
};

struct LineSegmentVertex
{
    glm::vec3 color;
    glm::vec3 position;
};

namespace std
{
    template<> struct hash<Vertex>
    {
        size_t operator()(Vertex const &vertex) const
        {
            return ((hash<glm::vec3>()(vertex.position)
                ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1)
                ^ (hash<glm::vec2>()(vertex.uv) << 1);
        }
    };
}

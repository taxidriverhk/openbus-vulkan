#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Engine/Mesh.h"

struct RoadObjectConfig;
class Image;

struct RoadInfo
{
    float length;
    float radius;

    float elevationStart;
    float elevationEnd;

    glm::vec3 position;
    float rotationZ;
};

struct Road
{
    uint32_t id;
    RoadInfo info;
    std::vector<Mesh> meshes;
};

class RoadLoader
{
public:
    RoadLoader();
    ~RoadLoader();

    bool LoadFromFile(
        const std::string &filename,
        const RoadInfo &info,
        Road &road);

private:
    static constexpr float SEGMENTS_PER_METER = 2.0f;
    static constexpr float TEXTURE_VERTICAL_INCREMENT_PER_METER = 0.1f;

    std::unordered_map<std::string, std::unique_ptr<RoadObjectConfig>> loadedRoadConfigs;
};

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Common/FileSystem.h"
#include "Common/Identifier.h"
#include "Common/Logger.h"
#include "Config/ConfigReader.h"
#include "Config/ObjectConfig.h"
#include "Engine/Image.h"
#include "Engine/Material.h"
#include "Road.h"

RoadLoader::RoadLoader()
{
}

RoadLoader::~RoadLoader()
{
}

bool RoadLoader::LoadFromFile(
    const std::string &filename,
    const RoadInfo &info,
    Road &road)
{
    // Read the config from cache if possible
    RoadObjectConfig roadObjectConfig;
    if (loadedRoadConfigs.count(filename) == 0)
    {
        if (!ConfigReader::ReadConfig(filename, roadObjectConfig))
        {
            Logger::Log(LogLevel::Error, "Failed to load road config from {}", filename);
            return false;
        }
        loadedRoadConfigs[filename] = std::make_unique<RoadObjectConfig>(roadObjectConfig);
    }
    else
    {
        roadObjectConfig = *loadedRoadConfigs[filename];
    }

    const float radius = info.radius;
    const float length = info.length;

    int segments;
    float radiansPerSegment;

    if (radius == 0.0f)
    {
        segments = 1;
        radiansPerSegment = 0.0f;
    }
    else
    {
        float centralAngle = length / radius;
        segments = (int) std::ceilf(length / SEGMENTS_PER_METER);
        radiansPerSegment = centralAngle / segments;
    }

    std::string roadBaseDirectory = FileSystem::GetParentDirectory(filename);

    road.meshes.resize(roadObjectConfig.meshes.size());
    for (uint32_t i = 0; i < roadObjectConfig.meshes.size(); i++)
    {
        const RoadMeshInfo &meshInfo = roadObjectConfig.meshes[i];

        // Map loader will be responsible for populating the ID for each mesh
        Mesh &roadMesh = road.meshes[i];
        
        const std::string &diffuseImagePath = FileSystem::GetTextureFile(roadBaseDirectory, meshInfo.material.diffuse);
        Image diffuseImage;
        if (!diffuseImage.Load(diffuseImagePath, ImageColor::ColorWithAlpha))
        {
            Logger::Log(LogLevel::Error, "Failed to load image from {}", diffuseImagePath);
            return false;
        }
        roadMesh.material = std::make_shared<Material>();
        roadMesh.material->id = Identifier::GenerateIdentifier(diffuseImagePath);
        roadMesh.material->diffuseImage = std::make_shared<Image>(diffuseImage);

        std::vector<Vertex> &vertices = roadMesh.vertices;
        std::vector<uint32_t> &indices = roadMesh.indices;

        float startX = meshInfo.startPosition.x,
              startZ = meshInfo.startPosition.y;
        float startU = meshInfo.startTextureCoord.x,
              startV = meshInfo.startTextureCoord.y;
        float endX = meshInfo.endPosition.x,
              endZ = meshInfo.endPosition.y;
        float endU = meshInfo.endTextureCoord.x,
              endV = meshInfo.endTextureCoord.y;
        for (int i = 0; i < segments; i++)
        {
            float currAngle = i * radiansPerSegment;
            float nextAngle = (i + 1) * radiansPerSegment;

            float cosCurrAngle = glm::cos(currAngle),
                  sinCurrAngle = glm::sin(currAngle);
            float cosNextAngle = glm::cos(nextAngle),
                  sinNextAngle = glm::sin(nextAngle);

            float currStartTextureCoordV = i * startV * TEXTURE_VERTICAL_INCREMENT_PER_METER,
                  currEndTextureCoordV = i * endV * TEXTURE_VERTICAL_INCREMENT_PER_METER;
            float nextStartTextureCoordV = (i + 1) * startV * TEXTURE_VERTICAL_INCREMENT_PER_METER,
                  nextEndTextureCoordV = (i + 1) * endV * TEXTURE_VERTICAL_INCREMENT_PER_METER;

            // Use the height for the normals for now
            glm::vec3 currNormal = glm::normalize(glm::vec3{ startZ, startZ, 2.0f });
            glm::vec3 nextNormal = glm::normalize(glm::vec3{ endZ, endZ, 2.0f });

            Vertex currStartVertex{};
            currStartVertex.position =
            {
                radius - (radius - startX) * cosCurrAngle,
                (radius + startX) * sinCurrAngle,
                startZ
            };
            currStartVertex.normal = currNormal;
            currStartVertex.uv = { startU, currStartTextureCoordV };

            Vertex currEndVertex{};
            currEndVertex.position =
            {
                radius - (radius - endX) * cosCurrAngle,
                (radius + endX) * sinCurrAngle,
                startZ
            };
            currEndVertex.normal = currNormal;
            currEndVertex.uv = { endU, currEndTextureCoordV };

            Vertex nextStartVertex{};
            nextStartVertex.position =
            {
                radius - (radius - startX) * cosNextAngle,
                (radius + startX) * sinNextAngle,
                endZ
            };
            nextStartVertex.normal = nextNormal;
            nextStartVertex.uv = { startU, nextStartTextureCoordV };

            Vertex nextEndVertex{};
            nextEndVertex.position =
            {
                radius - (radius - endX) * cosNextAngle,
                (radius + endX) * sinNextAngle,
                endZ
            };
            nextEndVertex.normal = nextNormal;
            nextStartVertex.uv = { endU, nextEndTextureCoordV };

            uint32_t startIndex = static_cast<uint32_t>(vertices.size());
            vertices.push_back(currStartVertex);
            vertices.push_back(currEndVertex);
            vertices.push_back(nextStartVertex);
            vertices.push_back(nextEndVertex);

            indices.push_back(startIndex);
            indices.push_back(startIndex + 1U);
            indices.push_back(startIndex + 2U);
            indices.push_back(startIndex + 2U);
            indices.push_back(startIndex + 1U);
            indices.push_back(startIndex + 3U);
        }
    }

    return true;
}

#include <assert.h>
#include <algorithm>
#include <execution>
#include <future>
#include <numeric>

#include "Common/Identifier.h"
#include "Common/Logger.h"
#include "Camera.h"
#include "DrawEngine.h"
#include "Entity.h"
#include "Image.h"
#include "Material.h"
#include "Renderer.h"
#include "Screen.h"
#include "Vulkan/VulkanDrawEngine.h"

Renderer::Renderer(Camera *camera)
    : camera(camera),
      drawEngine()
{
}

Renderer::~Renderer()
{
}

void Renderer::Cleanup()
{
    drawEngine->Destroy();
}

void Renderer::DrawScene()
{
    drawEngine->UpdateCamera(camera);
    drawEngine->DrawFrame();
}

void Renderer::Initialize(Screen *screen)
{
    assert(("Screen must be defined for the renderer", screen != nullptr));

    Logger::Log(LogLevel::Info, "Initializing graphics context");
#if _DEBUG
    bool enableDebugging = true;
#else
    bool enableDebugging = false;
#endif
    drawEngine = std::make_unique<VulkanDrawEngine>(screen, enableDebugging);
    drawEngine->Initialize();
}

void Renderer::PutText(const Text &text)
{
    ScreenMesh screenMesh;
    if (!fontManager.GenerateTextMesh(text, screenMesh))
    {
        Logger::Log(LogLevel::Warning, "Failed to add or update text with ID {}", text.id);
        return;
    }
    drawEngine->LoadScreenObject(screenMesh);
}

void Renderer::LoadBackground(const std::string &skyBoxImageFilePath, bool enableFog)
{
    // Load any resources that need to be loaded regardless of the game state
    // (ex. sky, sun, etc.)
    Logger::Log(LogLevel::Info, "Loading skybox into buffer");

    Image skyBoxImage(skyBoxImageFilePath);
    CubeMap cubeMap{};
    cubeMap.vertices.resize(8);
    cubeMap.vertices[0].position = { 1.0f, 1.0f, -1.0f };
    cubeMap.vertices[1].position = { 1.0f, -1.0f, -1.0f };
    cubeMap.vertices[2].position = { 1.0f, 1.0f, 1.0f };
    cubeMap.vertices[3].position = { 1.0f, -1.0f, 1.0f };
    cubeMap.vertices[4].position = { -1.0f, 1.0f, -1.0f };
    cubeMap.vertices[5].position = { -1.0f, -1.0f, -1.0f };
    cubeMap.vertices[6].position = { -1.0f, 1.0f, 1.0f };
    cubeMap.vertices[7].position = { -1.0f, -1.0f, 1.0f };

    float sizeMultiplier = camera->GetZFar();
    for (uint32_t i = 0; i < cubeMap.vertices.size(); i++)
    {
        cubeMap.vertices[i].position *= sizeMultiplier * 0.9f;
    }

    cubeMap.indices =
    {
        4, 2, 0,
        2, 7, 3,
        6, 5, 7,
        1, 7, 5,
        0, 3, 1,
        4, 1, 5,
        4, 6, 2,
        2, 6, 7,
        6, 4, 5,
        1, 3, 7,
        0, 2, 3,
        4, 0, 1
    };
    for (int i = 0; i < 6; i++)
    {
        cubeMap.images.push_back(&skyBoxImage);
    }

    drawEngine->LoadCubeMap(cubeMap);
    Logger::Log(LogLevel::Info, "Finished loading skybox into buffer");

    if (enableFog)
    {
        Logger::Log(LogLevel::Info, "Fog is enabled, setting fog density and gradient");
        drawEngine->SetFog(0.002f, camera->GetZFar() * 0.005f);
    }
    else
    {
        drawEngine->SetFog(0.0f, 5.0f);
    }
}

void Renderer::LoadBlock(uint32_t blockId, Terrain &terrain, std::vector<Entity> &entities)
{
    Logger::Log(LogLevel::Info, "Loading {} objects into buffer", entities.size());
    for (Entity &entity : entities)
    {
        drawEngine->LoadEntity(entity);
        blockIdEntityIdsMap[blockId].push_back(entity.id);
    }
    Logger::Log(LogLevel::Info, "Finished loading {} objects", entities.size());

    Logger::Log(LogLevel::Info, "Loading terrain into buffer");
    drawEngine->LoadTerrain(terrain);
    Logger::Log(LogLevel::Info, "Finished loading terrain into buffer");
}

void Renderer::RemoveText(uint32_t textMeshId)
{
    uint32_t screenMeshId = Identifier::GenerateIdentifier(IdentifierType::ScreenObject, textMeshId);
    drawEngine->UnloadScreenObject(screenMeshId);
}

void Renderer::UnloadEntities(uint32_t blockId)
{
    if (blockIdEntityIdsMap.count(blockId) == 0)
    {
        return;
    }

    std::list<uint32_t> &entityIds = blockIdEntityIdsMap[blockId];
    Logger::Log(LogLevel::Info, "Unloading {} objects from buffer", entityIds.size());
    for (uint32_t entityId : entityIds)
    {
        drawEngine->UnloadEntity(entityId);
    }
    Logger::Log(LogLevel::Info, "Finished unloading {} objects", entityIds.size());

    Logger::Log(LogLevel::Info, "Unloading terrain from buffer");
    drawEngine->UnloadTerrain(blockId);
    Logger::Log(LogLevel::Info, "Finished unloading terrain from buffer");

    blockIdEntityIdsMap.erase(blockId);
}

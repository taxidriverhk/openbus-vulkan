#include <assert.h>
#include <algorithm>
#include <execution>
#include <future>
#include <numeric>

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
      drawEngine(),
      meshLoader(),
      // TODO: read from configuration
      terrainLoader(1000, 10, 50, 50)
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

void Renderer::CreateContext(Screen *screen)
{
    assert(("Screen must be defined for the renderer", screen != nullptr));

#if _DEBUG
    bool enableDebugging = true;
#else
    bool enableDebugging = false;
#endif
    drawEngine = std::make_unique<VulkanDrawEngine>(screen, enableDebugging);
    drawEngine->Initialize();
}

void Renderer::LoadBackground()
{
    // Load any resources that need to be loaded regardless of the game state
    // (ex. sky, sun, etc.)
    Logger::Log(LogLevel::Info, "Loading skybox into buffer");
    Image skyBoxImage("sky.bmp");
    CubeMap cubeMap{};
    for (int i = 0; i < 6; i++)
    {
        cubeMap.images.push_back(&skyBoxImage);
    }
    drawEngine->LoadCubeMap(cubeMap);
    Logger::Log(LogLevel::Info, "Finished loading skybox into buffer");
}

void Renderer::LoadBlock(Terrain &terrain, std::vector<Entity> &entities)
{
    Logger::Log(LogLevel::Info, "Loading %d objects into buffer", entities.size());
    for (Entity &entityLoaded : entities)
    {
        drawEngine->LoadEntity(entityLoaded);
    }
    Logger::Log(LogLevel::Info, "Finished loading %d objects", entities.size());

    Logger::Log(LogLevel::Info, "Loading terrain into buffer");
    drawEngine->LoadTerrain(terrain);
    Logger::Log(LogLevel::Info, "Finished loading terrain into buffer");
}

void Renderer::UnloadEntities(uint32_t blockId)
{

}

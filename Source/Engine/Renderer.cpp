#include <assert.h>
#include <algorithm>
#include <execution>
#include <future>
#include <numeric>

#include "Common/Logger.h"
#include "Renderer.h"
#include "Vulkan/VulkanDrawEngine.h"

Renderer::Renderer(Camera *camera)
    : camera(camera),
      drawEngine(),
      meshLoader(),
      // TODO: read from configuration
      terrainLoader(1000, 10, 100, 500)
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

void Renderer::LoadScene()
{
    std::future<Terrain> terrainFuture = std::async(
        std::launch::async,
        [&]()
        {
            Terrain terrain;
            terrainLoader.LoadFromHeightMap("heightmap.png", terrain);
            return terrain;
        });

    uint32_t numberOfMeshes = 10;
    Logger::Log(LogLevel::Info, "Loading models and images from files");

    Material materials[] =
    {
        {
            1,
            std::make_shared<Image>("formula1.png"),
            nullptr,
            nullptr
        },
        {
            4,
            std::make_shared<Image>("wallpaper.bmp"),
            nullptr,
            nullptr
        }
    };

    std::mutex addMeshMutex;
    std::vector<Entity> entitiesLoaded;
    auto asyncLoadMeshInfoBuffer = [&](const int &index)
    {
        Mesh formula1{}, wallpaper;

        if (!meshLoader.LoadFromFile("formula1.obj", formula1)
            || !meshLoader.LoadFromFile("wallpaper.obj", wallpaper))
        {
            throw std::runtime_error("Failed to load models from files");
        }

        formula1.id = 1;
        formula1.material = std::make_shared<Material>(materials[0]);

        wallpaper.id = 2;
        wallpaper.material = std::make_shared<Material>(materials[1]);

        Entity entity1{};
        entity1.id = index;
        entity1.mesh = std::make_shared<Mesh>(formula1);
        entity1.translation = { index * 2.0f , 0.0f, 0.0f };
        entity1.scale = { 0.1f, 0.1f, 0.1f };

        Entity entity2{};
        entity2.id = index * 2;
        entity2.mesh = std::make_shared<Mesh>(wallpaper);
        entity2.translation = { index * 2.5f , 5.0f, 0.0f };
        entity2.scale = { 1.0f, 1.0f, 1.0f };

        addMeshMutex.lock();
        entitiesLoaded.push_back(entity1);
        entitiesLoaded.push_back(entity2);
        addMeshMutex.unlock();
    };

    std::vector<uint32_t> meshIndices(numberOfMeshes);
    std::iota(meshIndices.begin(), meshIndices.end(), 0);

    std::for_each(
        std::execution::par,
        meshIndices.begin(),
        meshIndices.end(),
        asyncLoadMeshInfoBuffer);
    Logger::Log(LogLevel::Info, "Loading %d objects into buffer", numberOfMeshes);
    for (Entity &entityLoaded : entitiesLoaded)
    {
        drawEngine->LoadEntity(entityLoaded);
    }
    Logger::Log(LogLevel::Info, "Finished loading %d objects", numberOfMeshes);

    Logger::Log(LogLevel::Info, "Loading skybox into buffer");
    Image skyBoxImage("sky.bmp");
    CubeMap cubeMap{};
    for (int i = 0; i < 6; i++)
    {
        cubeMap.images.push_back(&skyBoxImage);
    }
    drawEngine->LoadCubeMap(cubeMap);
    Logger::Log(LogLevel::Info, "Finished loading skybox into buffer");

    Logger::Log(LogLevel::Info, "Loading terrain into buffer");
    Terrain loadedTerrain = terrainFuture.get();
    drawEngine->LoadTerrain(loadedTerrain);
    Logger::Log(LogLevel::Info, "Finished loading terrain into buffer");
}

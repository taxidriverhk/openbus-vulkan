#include <algorithm>
#include <execution>
#include <numeric>

#include "Common/Logger.h"
#include "Renderer.h"
#include "Vulkan/VulkanDrawEngine.h"

Renderer::Renderer(Camera *camera)
    : camera(camera),
      meshLoader(),
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

void Renderer::CreateContext(Screen *screen)
{
#if _DEBUG
    drawEngine = std::make_unique<VulkanDrawEngine>(screen, true);
#else
    drawEngine = std::make_unique<VulkanDrawEngine>(screen, false);
#endif
    drawEngine->Initialize();
}

void Renderer::LoadScene()
{
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
        Mesh car = meshLoader.LoadFromFile("formula1.obj");
        car.id = 1;
        car.material = std::make_shared<Material>(materials[0]);

        Mesh formula1 = meshLoader.LoadFromFile("wallpaper.obj");
        formula1.id = 2;
        formula1.material = std::make_shared<Material>(materials[1]);

        Entity entity1{};
        entity1.id = index;
        entity1.mesh = std::make_shared<Mesh>(car);
        entity1.translation = { index * 2.0f , 0.0f, 0.0f };
        entity1.scale = { 0.1f, 0.1f, 0.1f };

        Entity entity2{};
        entity2.id = index * 2;
        entity2.mesh = std::make_shared<Mesh>(formula1);
        entity2.translation = { index * 2.0f , 5.0f, 0.0f };
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
        drawEngine->LoadIntoBuffer(entityLoaded);
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
}

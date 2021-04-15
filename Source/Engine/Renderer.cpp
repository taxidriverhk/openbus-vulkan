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
            std::make_shared<Image>("car-black.bmp"),
            nullptr,
            nullptr
        },
        {
            2,
            std::make_shared<Image>("car-yellow.bmp"),
            nullptr,
            nullptr
        },
        {
            3,
            std::make_shared<Image>("car-red.bmp"),
            nullptr,
            nullptr
        }
    };

    std::mutex addMeshMutex;
    std::vector<Entity> entitiesLoaded;
    auto asyncLoadMeshInfoBuffer = [&](const int &index)
    {
        Mesh car = meshLoader.LoadFromFile("car.obj");
        car.id = 1;
        car.material = std::make_shared<Material>(materials[index % 3]);

        Entity entity{};
        entity.id = index;
        entity.mesh = std::make_shared<Mesh>(car);
        entity.translation = { index * 2.0f , 0.0f, 0.0f };

        addMeshMutex.lock();
        entitiesLoaded.push_back(entity);
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

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

void Renderer::CreateContext(const std::unique_ptr<Screen> &screen)
{
#if _DEBUG
    drawEngine = std::make_unique<VulkanDrawEngine>(screen.get(), true);
#else
    drawEngine = std::make_unique<VulkanDrawEngine>(screen.get(), false);
#endif
    drawEngine->Initialize();
}

void Renderer::LoadScene()
{
    uint32_t numberOfMeshes = 1;
    Logger::Log(LogLevel::Info, "Loading %d objects into buffer", numberOfMeshes);

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
    std::vector<Mesh> meshesLoaded;
    auto asyncLoadMeshInfoBuffer = [&](const int &index)
    {
        Mesh car = meshLoader.LoadFromFile("car.obj");
        car.material = std::make_shared<Material>(materials[index % 3]);

        addMeshMutex.lock();
        meshesLoaded.push_back(car);
        addMeshMutex.unlock();
    };

    std::vector<uint32_t> meshIndices(numberOfMeshes);
    std::iota(meshIndices.begin(), meshIndices.end(), 0);

    std::for_each(
        std::execution::par,
        meshIndices.begin(),
        meshIndices.end(),
        asyncLoadMeshInfoBuffer);

    for (Mesh &meshLoaded : meshesLoaded)
    {
        drawEngine->LoadIntoBuffer(meshLoaded);
    }
}

#include <algorithm>
#include <execution>
#include <numeric>

#include "Common/Logger.h"
#include "Renderer.h"
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
    uint32_t numberOfMeshes = 20;
    Logger::Log(LogLevel::Info, "Loading %d hard-coded objects into buffer", numberOfMeshes);

    std::mutex addMeshMutex;
    std::vector<Mesh> meshesLoaded;
    auto asyncLoadMeshInfoBuffer = [&](const int &index)
    {
        Material material1{};
        material1.id = 1;
        material1.diffuseImage = std::make_shared<Image>("texture.jpg");

        Material material2{};
        material2.id = 2;
        material2.diffuseImage = std::make_shared<Image>("texture2.jpg");

        float offset = index * 0.5f;
        Mesh rectangle
        {
            1,
            {
                {{offset + -0.5f, offset + -0.5f, 0.0f - offset}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
                {{offset + 0.5f, offset + -0.5f, 0.0f - offset}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
                {{offset + 0.5f, offset + 0.5f, 0.0f - offset}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
                {{offset + -0.5f, offset + 0.5f, 0.0f - offset}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}
            },
            {
                0, 1, 2, 2, 3, 0
            },
            std::make_shared<Material>(index % 2 == 0 ? material1 : material2)
        };

        addMeshMutex.lock();
        meshesLoaded.push_back(rectangle);
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

#include <thread>

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
    // TODO: this part is hard-coded for testing only
    auto asyncLoadingFunction = [this]()
    {
        Logger::Log(LogLevel::Info, "Loading %d hard-coded objects into buffer", 2);
        uint32_t numberOfMeshes = 2;

        Material material1{};
        material1.id = 1;
        material1.diffuseImage = std::make_shared<Image>("texture.jpg");

        Material material2{};
        material2.id = 2;
        material2.diffuseImage = std::make_shared<Image>("texture2.jpg");

        for (uint32_t i = 0; i < numberOfMeshes; i++)
        {
            float offset = i * 0.5f;
            Mesh rectangles[] =
            {
                {
                    1,
                    {
                        {{offset + -0.5f, offset + -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
                        {{offset + 0.5f, offset + -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
                        {{offset + 0.5f, offset + 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
                        {{offset + -0.5f, offset + 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}
                    },
                    {
                        0, 1, 2, 2, 3, 0
                    },
                    std::make_shared<Material>(i % 2 == 0 ? material1 : material2)
                }
            };
            std::vector<Mesh> meshes(rectangles, rectangles + 1);
            drawEngine->LoadIntoBuffer(meshes);
        }
    };

    // TODO: need to create a secondary command pool and buffer for async loading
    //std::thread asyncLoadingThread(asyncLoadingFunction);
    //asyncLoadingThread.detach();
    asyncLoadingFunction();
}

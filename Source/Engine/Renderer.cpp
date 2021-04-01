#include "Renderer.h"
#include "Vulkan/VulkanDrawEngine.h"

Renderer::Renderer()
    : drawEngine()
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
    // TODO: remove the hard-coded vertices
    Mesh triangles[] =
    {
        {
            1,
            {
                {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
                {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
            },
            {
                0, 1, 2
            }
        },
        {
            2,
            {
                {{0.0f, -0.25f}, {1.0f, 0.0f, 0.0f}},
                {{0.75f, 0.75f}, {1.0f, 0.0f, 0.0f}},
                {{-0.75f, 0.75f}, {1.0f, 0.0f, 0.0f}}
            },
            {
                0, 1, 2
            }
        }
    };

    std::vector<Mesh> meshes(triangles, triangles + 2);
    drawEngine->LoadIntoBuffer(meshes);
}

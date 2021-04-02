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
    Mesh triangles[] =
    {
        {
            1,
            {
                {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
            },
            {
                0, 1, 2, 2, 3, 0
            }
        },
        {
            2,
            {
                {{0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                {{1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, 1.0f}, {1.0f, 0.0f, 0.0f}},
                {{0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}}
            },
            {
                0, 1, 2, 2, 3, 0
            }
        }
    };

    std::vector<Mesh> meshes(triangles, triangles + 2);

    uint32_t bufferId = 1;
    drawEngine->LoadIntoBuffer(1, meshes);
    camera->MoveTo(2.0f, 2.0f, 2.0f);
}

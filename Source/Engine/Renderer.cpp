#include "Renderer.h"
#include "Vulkan/VulkanDrawEngine.h";

Renderer::Renderer()
    : drawEngine()
{
}

Renderer::~Renderer()
{
}

void Renderer::Cleanup()
{
    drawEngine->DestroyContext();
}

void Renderer::DrawScene()
{

}

void Renderer::CreateContext(const std::unique_ptr<Screen> &screen)
{
#if _DEBUG
    drawEngine = std::make_unique<VulkanDrawEngine>(screen.get(), true);
#else
    drawEngine = std::make_unique<VulkanDrawEngine>(screen.get(), false);
#endif
    drawEngine->CreateContext();
}

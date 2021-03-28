#include "Renderer.h"

Renderer::Renderer()
    : vulkanInstance()
{
}

Renderer::~Renderer()
{
}

void Renderer::Cleanup()
{
    vulkanInstance->Destroy();
}

void Renderer::DrawScene()
{

}

void Renderer::PrepareContext(GLFWwindow *window)
{
#if _DEBUG
    vulkanInstance = std::make_unique<VulkanInstance>(window, true);
#else
    vulkanInstance = std::make_unique<VulkanInstance>(window, false);
#endif
    vulkanInstance->Create();
}

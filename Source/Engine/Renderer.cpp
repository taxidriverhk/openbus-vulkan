#include "Renderer.h"

Renderer::Renderer()
{

}

Renderer::~Renderer()
{

}

void Renderer::Cleanup()
{
    Vulkan::DestroyInstance(vulkanInstance);
}

void Renderer::DrawScene()
{

}

void Renderer::PrepareContext()
{
    Vulkan::CreateInstance(&vulkanInstance);
    Vulkan::CreatePipeline();
}

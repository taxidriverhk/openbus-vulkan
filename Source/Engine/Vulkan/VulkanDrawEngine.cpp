#include "VulkanDrawEngine.h"

VulkanDrawEngine::VulkanDrawEngine(Screen *screen, bool enableDebugging)
    : context(),
      screen(screen),
      enableDebugging(enableDebugging)
{
}

VulkanDrawEngine::~VulkanDrawEngine()
{
}

void VulkanDrawEngine::CreateContext()
{
    context = std::make_unique<VulkanContext>(screen->GetWindow(), enableDebugging);
    context->Create();
}

void VulkanDrawEngine::DestroyContext()
{
    context->Destroy();
}

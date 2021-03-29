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
    LoadShaders();
}

void VulkanDrawEngine::DestroyContext()
{
    vertexShader->Unload();
    fragmentShader->Unload();
    context->Destroy();
}

void VulkanDrawEngine::LoadShaders()
{
    // TODO: test code to verify that the drawing works
    vertexShader = std::make_unique<VulkanShader>(context.get());
    vertexShader->Compile("test_vertex_shader.glsl");
    vertexShader->Load();

    fragmentShader = std::make_unique<VulkanShader>(context.get());
    fragmentShader->Compile("test_fragment_shader.glsl");
    fragmentShader->Load();
}

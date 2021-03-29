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
    CreatePipeline();
}

void VulkanDrawEngine::DestroyContext()
{
    pipeline->Destroy();
    vertexShader->Unload();
    fragmentShader->Unload();
    context->Destroy();
}

void VulkanDrawEngine::CreatePipeline()
{
    // TODO: test code to verify that the drawing works
    vertexShader = std::make_unique<VulkanShader>(context.get(), VulkanShaderType::VERTEX);
    vertexShader->Compile("test_vertex_shader.glsl");
    vertexShader->Load();

    fragmentShader = std::make_unique<VulkanShader>(context.get(), VulkanShaderType::FRAGMENT);
    fragmentShader->Compile("test_fragment_shader.glsl");
    fragmentShader->Load();

    pipeline = std::make_unique<VulkanPipeline>(context.get());
    pipeline->Create(*vertexShader.get(), *fragmentShader.get());
}

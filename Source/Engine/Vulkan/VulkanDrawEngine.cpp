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

void VulkanDrawEngine::Destroy()
{
    context->WaitIdle();

    bufferManager->Destroy();
    pipeline->Destroy();
    vertexShader->Unload();
    fragmentShader->Unload();
    context->Destroy();
}

void VulkanDrawEngine::DrawFrame()
{
    uint32_t imageIndex;
    bufferManager->BeginFrame(imageIndex);
    bufferManager->Submit(imageIndex);
    bufferManager->EndFrame(imageIndex);
}

void VulkanDrawEngine::Initialize()
{
    context = std::make_unique<VulkanContext>(screen, enableDebugging);
    context->Create();

    CreatePipeline();
    CreateBuffer();
}

void VulkanDrawEngine::CreateBuffer()
{
    bufferManager = std::make_unique<VulkanBufferManager>(context.get(), pipeline.get());
    bufferManager->Create();
}

void VulkanDrawEngine::CreatePipeline()
{
    // TODO: test code to verify that the drawing works
    vertexShader = std::make_unique<VulkanShader>(context.get(), VulkanShaderType::Vertex);
    vertexShader->Compile("test_vertex_shader.glsl");
    vertexShader->Load();

    fragmentShader = std::make_unique<VulkanShader>(context.get(), VulkanShaderType::Fragment);
    fragmentShader->Compile("test_fragment_shader.glsl");
    fragmentShader->Load();

    pipeline = std::make_unique<VulkanPipeline>(
        context.get(),
        vertexShader.get(),
        fragmentShader.get());
    pipeline->Create();
}

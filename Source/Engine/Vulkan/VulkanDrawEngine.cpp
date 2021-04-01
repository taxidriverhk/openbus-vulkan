#include "Engine/Mesh.h"
#include "VulkanDrawEngine.h"

VulkanDrawEngine::VulkanDrawEngine(Screen *screen, bool enableDebugging)
    : context(),
      screen(screen),
      enableDebugging(enableDebugging),
      vertexBufferIds()
{
}

VulkanDrawEngine::~VulkanDrawEngine()
{
}

void VulkanDrawEngine::Destroy()
{
    ClearBuffers();

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
    bufferManager->Draw(imageIndex);
}

void VulkanDrawEngine::Initialize()
{
    context = std::make_unique<VulkanContext>(screen, enableDebugging);
    context->Create();

    CreatePipeline();
    CreateBuffer();
}

void VulkanDrawEngine::ClearBuffers()
{
    for (uint32_t vertexBufferId : vertexBufferIds)
    {
        bufferManager->UnloadBuffer(vertexBufferId);
    }
    vertexBufferIds.clear();
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

void VulkanDrawEngine::LoadIntoBuffer(std::vector<Mesh> &meshes)
{
    for (Mesh mesh : meshes)
    {
        // TODO: use something else as the buffer ID
        bufferManager->LoadVertices(mesh.id, mesh.vertices, mesh.indices);
        vertexBufferIds.insert(mesh.id);
    }
}

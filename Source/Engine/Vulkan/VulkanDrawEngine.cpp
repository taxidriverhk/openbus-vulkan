#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Common/Logger.h"
#include "Engine/Camera.h"
#include "Engine/Mesh.h"
#include "VulkanDrawEngine.h"

VulkanDrawEngine::VulkanDrawEngine(Screen *screen, bool enableDebugging)
    : context(),
      screen(screen),
      enableDebugging(enableDebugging),
      staticBufferIds()
{
}

VulkanDrawEngine::~VulkanDrawEngine()
{
}

void VulkanDrawEngine::Destroy()
{
    context->WaitIdle();

    ClearBuffers();
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
    for (uint32_t vertexBufferId : staticBufferIds)
    {
        bufferManager->UnloadBuffer(vertexBufferId);
    }
    staticBufferIds.clear();
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
    fragmentShader = std::make_unique<VulkanShader>(context.get(), VulkanShaderType::Fragment);

    if (!vertexShader->Compile("shaders/test_vertex_shader.glsl")
        || !fragmentShader->Compile("shaders/test_fragment_shader.glsl"))
    {
        std::runtime_error("Failed to compile shader code");
    }
    
    vertexShader->Load();
    fragmentShader->Load();

    pipeline = std::make_unique<VulkanPipeline>(
        context.get(),
        vertexShader.get(),
        fragmentShader.get());
    pipeline->Create();
}

void VulkanDrawEngine::LoadIntoBuffer(Mesh &mesh)
{
    // TODO: need to do coordinate conversion here
    uint32_t bufferId = bufferManager->LoadIntoBuffer(mesh.vertices, mesh.indices, mesh.material.get());
    staticBufferIds.insert(bufferId);
}

void VulkanDrawEngine::UpdateCamera(Camera *camera)
{
    glm::vec3 position = ConvertToVulkanCoordinates(camera->GetPosition());
    glm::vec3 target = ConvertToVulkanCoordinates(camera->GetTarget());
    glm::vec3 up = ConvertToVulkanCoordinates(camera->GetUp());

    // Conversions required for Vulkan depth range
    VulkanUniformBufferInput input{};
    input.model = glm::identity<glm::mat4>();
    input.projection = glm::perspective(
        camera->GetFieldOfView(),
        camera->GetAspect(),
        camera->GetZNear(),
        camera->GetZFar());
    input.projection[1][1] *= -1;
    input.view = glm::lookAt(position, target, up);

    bufferManager->UpdateUniformBuffer(input);
}

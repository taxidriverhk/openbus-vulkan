#define GLM_DEPTH_ZERO_TO_ONE
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

void VulkanDrawEngine::LoadIntoBuffer(uint32_t bufferId, std::vector<Mesh> &meshes)
{
    // TODO: need to do coordinate conversion here
    std::vector<Vertex> combinedVertices;
    std::vector<uint32_t> combinedIndices;
    for (Mesh mesh : meshes)
    {
        uint32_t indexOffset = static_cast<uint32_t>(combinedVertices.size());
        combinedVertices.insert(combinedVertices.end(), mesh.vertices.begin(), mesh.vertices.end());
        for (uint32_t index : mesh.indices)
        {
            combinedIndices.push_back(index + indexOffset);
        }
    }
    bufferManager->LoadIntoBuffer(bufferId, combinedVertices, combinedIndices);
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

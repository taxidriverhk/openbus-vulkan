#include <algorithm>
#include <execution>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Common/Logger.h"
#include "Engine/Camera.h"
#include "Engine/Entity.h"
#include "VulkanDrawEngine.h"

VulkanDrawEngine::VulkanDrawEngine(Screen *screen, bool enableDebugging)
    : context(),
      screen(screen),
      enableDebugging(enableDebugging),
      bufferIds()
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
    for (uint32_t bufferId : bufferIds)
    {
        bufferManager->UnloadBuffer(bufferId);
    }
    bufferIds.clear();
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

void VulkanDrawEngine::LoadIntoBuffer(Entity &entity)
{
    std::shared_ptr<Mesh> mesh = entity.mesh;
    glm::vec3 translation = entity.translation;

    std::vector<Vertex> vertices = mesh->vertices;
    std::vector<Vertex> transformedVertices(vertices.size());
    std::transform(
        std::execution::par,
        vertices.begin(),
        vertices.end(),
        transformedVertices.begin(),
        [&](Vertex &vertex)
        {
            return ConvertToVulkanVertex(vertex);
        });

    glm::mat4 identitiyMatrix = glm::identity<glm::mat4>();
    glm::mat4 translatedMatrix = glm::translate(identitiyMatrix, translation);

    VulkanInstanceBufferInput instanceBufferInput{};
    instanceBufferInput.transformation = translatedMatrix;

    uint32_t bufferId = bufferManager->LoadIntoBuffer(
        mesh->id,
        mesh->material->id,
        instanceBufferInput,
        transformedVertices,
        mesh->indices,
        mesh->material.get());
    bufferIds.insert(bufferId);
}

void VulkanDrawEngine::UpdateCamera(Camera *camera)
{
    glm::vec3 position = ConvertToVulkanCoordinates(camera->GetPosition());
    glm::vec3 target = ConvertToVulkanCoordinates(camera->GetTarget());
    glm::vec3 up = ConvertToVulkanCoordinates(camera->GetUp());

    VulkanUniformBufferInput input{};
    input.model = glm::identity<glm::mat4>();
    input.projection = glm::perspective(
        camera->GetFieldOfView(),
        camera->GetAspect(),
        camera->GetZNear(),
        camera->GetZFar());
    // Conversion required for Vulkan depth range
    input.projection[1][1] *= -1;
    input.view = glm::lookAt(position, target, up);

    bufferManager->UpdateUniformBuffer(input);
}

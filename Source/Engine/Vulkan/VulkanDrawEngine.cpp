#include <algorithm>
#include <execution>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Common/Logger.h"
#include "Engine/Camera.h"
#include "Engine/Entity.h"
#include "VulkanCommon.h"
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

    DestroyPipelines();

    renderPass->Destroy();
    context->Destroy();
}

void VulkanDrawEngine::DestroyPipelines()
{
    cubeMapPipeline->Destroy();
    staticPipeline->Destroy();
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

    renderPass = std::make_unique<VulkanRenderPass>(context.get());
    renderPass->Create();

    CreatePipelines();
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
    VulkanDrawingPipelines pipelines{};
    pipelines.staticPipeline = staticPipeline.get();
    pipelines.cubeMapPipeline = cubeMapPipeline.get();

    bufferManager = std::make_unique<VulkanBufferManager>(context.get(), renderPass.get(), pipelines);
    bufferManager->Create();
}

void VulkanDrawEngine::CreatePipelines()
{
    VulkanShader staticVertexShader(context.get(), VulkanShaderType::Vertex);
    VulkanShader staticFragmentShader(context.get(), VulkanShaderType::Fragment);
    if (!staticVertexShader.Compile(STATIC_PIPELINE_VERTEX_SHADER)
        || !staticFragmentShader.Compile(STATIC_PIPELINE_FRAGMENT_SHADER))
    {
        throw std::runtime_error("Failed to compile static scene shader code");
    }
    staticVertexShader.Load();
    staticFragmentShader.Load();

    VulkanShader cubeMapVertexShader(context.get(), VulkanShaderType::Vertex);
    VulkanShader cubeMapFragmentShader(context.get(), VulkanShaderType::Fragment);
    if (!cubeMapVertexShader.Compile(CUBEMAP_PIPELINE_VERTEX_SHADER)
        || !cubeMapFragmentShader.Compile(CUBEMAP_PIPELINE_FRAGMENT_SHADER))
    {
        throw std::runtime_error("Failed to compile static scene shader code");
    }
    cubeMapVertexShader.Load();
    cubeMapFragmentShader.Load();

    VulkanPipelineConfig staticPipelineConfig{};
    staticPipelineConfig.vertexShader = &staticVertexShader;
    staticPipelineConfig.fragmentShader = &staticFragmentShader;
    staticPipelineConfig.cullMode = VK_CULL_MODE_BACK_BIT;
    staticPipelineConfig.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    staticPipeline = std::make_unique<VulkanPipeline>(context.get(), renderPass.get());
    staticPipeline->Create(staticPipelineConfig);

    VulkanPipelineConfig cubeMapPipelineConfig{};
    cubeMapPipelineConfig.vertexShader = &cubeMapVertexShader;
    cubeMapPipelineConfig.fragmentShader = &cubeMapFragmentShader;
    cubeMapPipelineConfig.cullMode = VK_CULL_MODE_NONE;
    cubeMapPipelineConfig.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    cubeMapPipeline = std::make_unique<VulkanPipeline>(context.get(), renderPass.get());
    cubeMapPipeline->Create(cubeMapPipelineConfig);

    staticVertexShader.Unload();
    staticFragmentShader.Unload();
    cubeMapVertexShader.Unload();
    cubeMapFragmentShader.Unload();
}

void VulkanDrawEngine::LoadCubeMap(CubeMap &cubeMap)
{
    bufferManager->UpdateCubeMapImage(cubeMap.images);
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
    input.projection = glm::perspective(
        camera->GetFieldOfView(),
        camera->GetAspect(),
        camera->GetZNear(),
        camera->GetZFar());
    // Conversion required for Vulkan depth range
    input.projection[1][1] *= -1;
    input.view = glm::lookAt(position, target, up);
    input.lightPosition = { 10.0f, 10.0f, 10.0f };
    input.eyePosition = position;

    bufferManager->UpdateUniformBuffer(input);
}

#include "Engine/Vulkan/VulkanCommon.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/VulkanRenderPass.h"
#include "Engine/Vulkan/VulkanShader.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineManager.h"

VulkanPipelineManager::VulkanPipelineManager(VulkanContext *context, VulkanRenderPass *renderPass)
    : context(context),
      renderPass(renderPass)
{
}

VulkanPipelineManager::~VulkanPipelineManager()
{
}

void VulkanPipelineManager::Create()
{
    VulkanShader staticVertexShader(context, VulkanShaderType::Vertex);
    VulkanShader staticFragmentShader(context, VulkanShaderType::Fragment);
    if (!staticVertexShader.Compile(STATIC_PIPELINE_VERTEX_SHADER)
        || !staticFragmentShader.Compile(STATIC_PIPELINE_FRAGMENT_SHADER))
    {
        throw std::runtime_error("Failed to compile static scene shader code");
    }
    staticVertexShader.Load();
    staticFragmentShader.Load();

    VulkanShader cubeMapVertexShader(context, VulkanShaderType::Vertex);
    VulkanShader cubeMapFragmentShader(context, VulkanShaderType::Fragment);
    if (!cubeMapVertexShader.Compile(CUBEMAP_PIPELINE_VERTEX_SHADER)
        || !cubeMapFragmentShader.Compile(CUBEMAP_PIPELINE_FRAGMENT_SHADER))
    {
        throw std::runtime_error("Failed to compile cube map shader code");
    }
    cubeMapVertexShader.Load();
    cubeMapFragmentShader.Load();

    VulkanShader terrainVertexShader(context, VulkanShaderType::Vertex);
    VulkanShader terrainFragmentShader(context, VulkanShaderType::Fragment);
    if (!terrainVertexShader.Compile(TERRAIN_PIPELINE_VERTEX_SHADER)
        || !terrainFragmentShader.Compile(TERRAIN_PIPELINE_FRAGMENT_SHADER))
    {
        throw std::runtime_error("Failed to compile terrain shader code");
    }
    terrainVertexShader.Load();
    terrainFragmentShader.Load();

    VulkanShader screenVertexShader(context, VulkanShaderType::Vertex);
    VulkanShader screenFragmentShader(context, VulkanShaderType::Fragment);
    if (!screenVertexShader.Compile(SCREEN_PIPELINE_VERTEX_SHADER)
        || !screenFragmentShader.Compile(SCREEN_PIPELINE_FRAGMENT_SHADER))
    {
        throw std::runtime_error("Failed to compile screen shader code");
    }
    screenVertexShader.Load();
    screenFragmentShader.Load();

    VulkanPipelineConfig staticPipelineConfig{};
    staticPipelineConfig.vertexShader = &staticVertexShader;
    staticPipelineConfig.fragmentShader = &staticFragmentShader;
    staticPipelineConfig.cullMode = VK_CULL_MODE_BACK_BIT;
    staticPipelineConfig.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    staticPipelineConfig.descriptorLayoutConfigs.resize(3);
    staticPipelineConfig.descriptorLayoutConfigs[0].type = VulkanDescriptorLayoutType::Uniform;
    staticPipelineConfig.descriptorLayoutConfigs[0].bindingCount = STATIC_PIPELINE_UNIFORM_DESCRIPTOR_LAYOUT_BINDING_COUNT;
    staticPipelineConfig.descriptorLayoutConfigs[0].bindings = STATIC_PIPELINE_UNIFORM_DESCRIPTOR_LAYOUT_BINDINGS;
    staticPipelineConfig.descriptorLayoutConfigs[1].type = VulkanDescriptorLayoutType::Image;
    staticPipelineConfig.descriptorLayoutConfigs[1].bindingCount = STATIC_PIPELINE_IMAGE_DESCRIPTOR_LAYOUT_BINDING_COUNT;
    staticPipelineConfig.descriptorLayoutConfigs[1].bindings = STATIC_PIPELINE_IMAGE_DESCRIPTOR_LAYOUT_BINDINGS;
    staticPipelineConfig.descriptorLayoutConfigs[2].type = VulkanDescriptorLayoutType::Instance;
    staticPipelineConfig.descriptorLayoutConfigs[2].bindingCount = STATIC_PIPELINE_INSTANCE_DESCRIPTOR_LAYOUT_BINDING_COUNT;
    staticPipelineConfig.descriptorLayoutConfigs[2].bindings = STATIC_PIPELINE_INSTANCE_DESCRIPTOR_LAYOUT_BINDINGS;

    staticPipelineConfig.vertexLayoutConfig.vertexSize = sizeof(Vertex);
    staticPipelineConfig.vertexLayoutConfig.descriptionCount = VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_COUNT;
    staticPipelineConfig.vertexLayoutConfig.descriptions = VERTEX_INPUT_ATTRIBUTE_DESCRIPTIONS;

    staticPipeline = std::make_unique<VulkanPipeline>(context, renderPass);
    staticPipeline->Create(staticPipelineConfig);

    VulkanPipelineConfig cubeMapPipelineConfig{};
    cubeMapPipelineConfig.vertexShader = &cubeMapVertexShader;
    cubeMapPipelineConfig.fragmentShader = &cubeMapFragmentShader;
    cubeMapPipelineConfig.cullMode = VK_CULL_MODE_NONE;
    cubeMapPipelineConfig.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    cubeMapPipelineConfig.descriptorLayoutConfigs.resize(2);
    cubeMapPipelineConfig.descriptorLayoutConfigs[0].type = VulkanDescriptorLayoutType::Uniform;
    cubeMapPipelineConfig.descriptorLayoutConfigs[0].bindingCount = STATIC_PIPELINE_UNIFORM_DESCRIPTOR_LAYOUT_BINDING_COUNT;
    cubeMapPipelineConfig.descriptorLayoutConfigs[0].bindings = STATIC_PIPELINE_UNIFORM_DESCRIPTOR_LAYOUT_BINDINGS;
    cubeMapPipelineConfig.descriptorLayoutConfigs[1].type = VulkanDescriptorLayoutType::Image;
    cubeMapPipelineConfig.descriptorLayoutConfigs[1].bindingCount = STATIC_PIPELINE_IMAGE_DESCRIPTOR_LAYOUT_BINDING_COUNT;
    cubeMapPipelineConfig.descriptorLayoutConfigs[1].bindings = STATIC_PIPELINE_IMAGE_DESCRIPTOR_LAYOUT_BINDINGS;

    cubeMapPipelineConfig.vertexLayoutConfig.vertexSize = sizeof(Vertex);
    cubeMapPipelineConfig.vertexLayoutConfig.descriptionCount = VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_COUNT;
    cubeMapPipelineConfig.vertexLayoutConfig.descriptions = VERTEX_INPUT_ATTRIBUTE_DESCRIPTIONS;

    cubeMapPipeline = std::make_unique<VulkanPipeline>(context, renderPass);
    cubeMapPipeline->Create(cubeMapPipelineConfig);

    VulkanPipelineConfig terrainPipelineConfig{};
    terrainPipelineConfig.vertexShader = &terrainVertexShader;
    terrainPipelineConfig.fragmentShader = &terrainFragmentShader;
    terrainPipelineConfig.cullMode = VK_CULL_MODE_BACK_BIT;
    terrainPipelineConfig.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    terrainPipelineConfig.descriptorLayoutConfigs.resize(2);
    terrainPipelineConfig.descriptorLayoutConfigs[0].type = VulkanDescriptorLayoutType::Uniform;
    terrainPipelineConfig.descriptorLayoutConfigs[0].bindingCount = STATIC_PIPELINE_UNIFORM_DESCRIPTOR_LAYOUT_BINDING_COUNT;
    terrainPipelineConfig.descriptorLayoutConfigs[0].bindings = STATIC_PIPELINE_UNIFORM_DESCRIPTOR_LAYOUT_BINDINGS;
    terrainPipelineConfig.descriptorLayoutConfigs[1].type = VulkanDescriptorLayoutType::Image;
    terrainPipelineConfig.descriptorLayoutConfigs[1].bindingCount = STATIC_PIPELINE_IMAGE_DESCRIPTOR_LAYOUT_BINDING_COUNT;
    terrainPipelineConfig.descriptorLayoutConfigs[1].bindings = STATIC_PIPELINE_IMAGE_DESCRIPTOR_LAYOUT_BINDINGS;

    terrainPipelineConfig.vertexLayoutConfig.vertexSize = sizeof(Vertex);
    terrainPipelineConfig.vertexLayoutConfig.descriptionCount = VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_COUNT;
    terrainPipelineConfig.vertexLayoutConfig.descriptions = VERTEX_INPUT_ATTRIBUTE_DESCRIPTIONS;

    terrainPipeline = std::make_unique<VulkanPipeline>(context, renderPass);
    terrainPipeline->Create(terrainPipelineConfig);

    VulkanPipelineConfig screenPipelineConfig{};
    screenPipelineConfig.vertexShader = &screenVertexShader;
    screenPipelineConfig.fragmentShader = &screenFragmentShader;
    screenPipelineConfig.cullMode = VK_CULL_MODE_NONE;
    screenPipelineConfig.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    screenPipelineConfig.descriptorLayoutConfigs.resize(2);
    screenPipelineConfig.descriptorLayoutConfigs[0].type = VulkanDescriptorLayoutType::Uniform;
    screenPipelineConfig.descriptorLayoutConfigs[0].bindingCount = STATIC_PIPELINE_UNIFORM_DESCRIPTOR_LAYOUT_BINDING_COUNT;
    screenPipelineConfig.descriptorLayoutConfigs[0].bindings = STATIC_PIPELINE_UNIFORM_DESCRIPTOR_LAYOUT_BINDINGS;
    screenPipelineConfig.descriptorLayoutConfigs[1].type = VulkanDescriptorLayoutType::Image;
    screenPipelineConfig.descriptorLayoutConfigs[1].bindingCount = STATIC_PIPELINE_IMAGE_DESCRIPTOR_LAYOUT_BINDING_COUNT;
    screenPipelineConfig.descriptorLayoutConfigs[1].bindings = STATIC_PIPELINE_IMAGE_DESCRIPTOR_LAYOUT_BINDINGS;

    screenPipelineConfig.vertexLayoutConfig.vertexSize = sizeof(ScreenObjectVertex);
    screenPipelineConfig.vertexLayoutConfig.descriptionCount = SCREEN_OBJECT_INPUT_ATTRIBUTE_DESCRIPTION_COUNT;
    screenPipelineConfig.vertexLayoutConfig.descriptions = SCREEN_OBJECT_INPUT_ATTRIBUTE_DESCRIPTIONS;

    screenPipeline = std::make_unique<VulkanPipeline>(context, renderPass);
    screenPipeline->Create(screenPipelineConfig);

    staticVertexShader.Unload();
    staticFragmentShader.Unload();
    cubeMapVertexShader.Unload();
    cubeMapFragmentShader.Unload();
    terrainVertexShader.Unload();
    terrainFragmentShader.Unload();
    screenVertexShader.Unload();
    screenFragmentShader.Unload();
}

void VulkanPipelineManager::Destroy()
{
    cubeMapPipeline->Destroy();
    staticPipeline->Destroy();
    terrainPipeline->Destroy();
    screenPipeline->Destroy();
}

VulkanDrawingPipelines VulkanPipelineManager::GetDrawingPipelines() const
{
    VulkanDrawingPipelines drawingPipelines{};
    drawingPipelines.staticPipeline = staticPipeline.get();
    drawingPipelines.cubeMapPipeline = cubeMapPipeline.get();
    drawingPipelines.terrainPipeline = terrainPipeline.get();
    drawingPipelines.screenPipeline = screenPipeline.get();
    return drawingPipelines;
}

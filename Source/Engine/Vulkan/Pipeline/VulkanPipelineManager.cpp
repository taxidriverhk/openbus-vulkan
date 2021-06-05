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

    VulkanShader lineVertexShader(context, VulkanShaderType::Vertex);
    VulkanShader lineFragmentShader(context, VulkanShaderType::Fragment);
    if (!lineVertexShader.Compile(LINE_PIPELINE_VERTEX_SHADER)
        || !lineFragmentShader.Compile(LINE_PIPELINE_FRAGMENT_SHADER))
    {
        throw std::runtime_error("Failed to compile line shader code");
    }
    lineVertexShader.Load();
    lineFragmentShader.Load();

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
    staticPipelineConfig.polygonMode = VK_POLYGON_MODE_FILL;
    staticPipelineConfig.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    staticPipelineConfig.depthTestEnable = true;
    staticPipelineConfig.enableColorBlending = true;

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

    staticPipelineConfig.pushConstantConfigs.resize(1);
    staticPipelineConfig.pushConstantConfigs[0].size = sizeof(VulkanMeshPushConstant);
    staticPipelineConfig.pushConstantConfigs[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

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
    cubeMapPipelineConfig.polygonMode = VK_POLYGON_MODE_FILL;
    cubeMapPipelineConfig.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    cubeMapPipelineConfig.depthTestEnable = true;
    cubeMapPipelineConfig.enableColorBlending = true;

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

    VulkanPipelineConfig linePipelineConfig{};
    linePipelineConfig.vertexShader = &lineVertexShader;
    linePipelineConfig.fragmentShader = &lineFragmentShader;
    linePipelineConfig.cullMode = VK_CULL_MODE_NONE;
    linePipelineConfig.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    linePipelineConfig.polygonMode = VK_POLYGON_MODE_LINE;
    linePipelineConfig.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    linePipelineConfig.depthTestEnable = true;
    linePipelineConfig.enableColorBlending = true;

    linePipelineConfig.descriptorLayoutConfigs.resize(1);
    linePipelineConfig.descriptorLayoutConfigs[0].type = VulkanDescriptorLayoutType::Uniform;
    linePipelineConfig.descriptorLayoutConfigs[0].bindingCount = STATIC_PIPELINE_UNIFORM_DESCRIPTOR_LAYOUT_BINDING_COUNT;
    linePipelineConfig.descriptorLayoutConfigs[0].bindings = STATIC_PIPELINE_UNIFORM_DESCRIPTOR_LAYOUT_BINDINGS;

    linePipelineConfig.vertexLayoutConfig.vertexSize = sizeof(LineSegmentVertex);
    linePipelineConfig.vertexLayoutConfig.descriptionCount = LINE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_COUNT;
    linePipelineConfig.vertexLayoutConfig.descriptions = LINE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTIONS;

    linePipeline = std::make_unique<VulkanPipeline>(context, renderPass);
    linePipeline->Create(linePipelineConfig);

    VulkanPipelineConfig terrainPipelineConfig{};
    terrainPipelineConfig.vertexShader = &terrainVertexShader;
    terrainPipelineConfig.fragmentShader = &terrainFragmentShader;
    terrainPipelineConfig.cullMode = VK_CULL_MODE_BACK_BIT;
    terrainPipelineConfig.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    terrainPipelineConfig.polygonMode = VK_POLYGON_MODE_FILL;
    terrainPipelineConfig.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    terrainPipelineConfig.depthTestEnable = true;
    terrainPipelineConfig.enableColorBlending = true;

    terrainPipelineConfig.descriptorLayoutConfigs.resize(2);
    terrainPipelineConfig.descriptorLayoutConfigs[0].type = VulkanDescriptorLayoutType::Uniform;
    terrainPipelineConfig.descriptorLayoutConfigs[0].bindingCount = STATIC_PIPELINE_UNIFORM_DESCRIPTOR_LAYOUT_BINDING_COUNT;
    terrainPipelineConfig.descriptorLayoutConfigs[0].bindings = STATIC_PIPELINE_UNIFORM_DESCRIPTOR_LAYOUT_BINDINGS;
    terrainPipelineConfig.descriptorLayoutConfigs[1].type = VulkanDescriptorLayoutType::Image;
    terrainPipelineConfig.descriptorLayoutConfigs[1].bindingCount = STATIC_PIPELINE_IMAGE_DESCRIPTOR_LAYOUT_BINDING_COUNT;
    terrainPipelineConfig.descriptorLayoutConfigs[1].bindings = STATIC_PIPELINE_IMAGE_DESCRIPTOR_LAYOUT_BINDINGS;

    terrainPipelineConfig.pushConstantConfigs.resize(1);
    terrainPipelineConfig.pushConstantConfigs[0].size = sizeof(VulkanMeshPushConstant);
    terrainPipelineConfig.pushConstantConfigs[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

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
    screenPipelineConfig.polygonMode = VK_POLYGON_MODE_FILL;
    screenPipelineConfig.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    screenPipelineConfig.depthTestEnable = false;
    screenPipelineConfig.enableColorBlending = true;

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
    lineVertexShader.Unload();
    lineFragmentShader.Unload();
    terrainVertexShader.Unload();
    terrainFragmentShader.Unload();
    screenVertexShader.Unload();
    screenFragmentShader.Unload();
}

void VulkanPipelineManager::Destroy()
{
    cubeMapPipeline->Destroy();
    staticPipeline->Destroy();
    linePipeline->Destroy();
    terrainPipeline->Destroy();
    screenPipeline->Destroy();
}

VulkanDrawingPipelines VulkanPipelineManager::GetDrawingPipelines() const
{
    VulkanDrawingPipelines drawingPipelines{};
    drawingPipelines.staticPipeline = staticPipeline.get();
    drawingPipelines.cubeMapPipeline = cubeMapPipeline.get();
    drawingPipelines.linePipeline = linePipeline.get();
    drawingPipelines.terrainPipeline = terrainPipeline.get();
    drawingPipelines.screenPipeline = screenPipeline.get();
    return drawingPipelines;
}

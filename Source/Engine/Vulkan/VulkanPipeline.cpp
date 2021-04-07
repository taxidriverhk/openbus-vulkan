#include "Engine/Mesh.h"
#include "VulkanCommon.h"
#include "VulkanPipeline.h"

VulkanPipeline::VulkanPipeline(VulkanContext *context, VulkanShader *vertexShader, VulkanShader *fragmentShader)
    : context(context),
      uniformDescriptorSetLayout(),
      perObjectDescriptorSetLayout(),
      fragmentShader(fragmentShader),
      vertexShader(vertexShader),
      pipeline(),
      pipelineLayout()
{
}

VulkanPipeline::~VulkanPipeline()
{
}

void VulkanPipeline::Create()
{
    VkDescriptorSetLayoutCreateInfo uniformDescriptorSetLayoutInfo{};
    uniformDescriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    uniformDescriptorSetLayoutInfo.bindingCount = STATIC_PIPELINE_UNIFORM_DESCRIPTOR_LAYOUT_BINDING_COUNT;
    uniformDescriptorSetLayoutInfo.pBindings = STATIC_PIPELINE_UNIFORM_DESCRIPTOR_LAYOUT_BINDINGS;
    ASSERT_VK_RESULT_SUCCESS(
        vkCreateDescriptorSetLayout(context->GetLogicalDevice(), &uniformDescriptorSetLayoutInfo, nullptr, &uniformDescriptorSetLayout),
        "Failed to create uniform descriptor set layout");

    VkDescriptorSetLayoutCreateInfo perObjectDescriptorSetLayoutInfo{};
    perObjectDescriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    perObjectDescriptorSetLayoutInfo.bindingCount = STATIC_PIPELINE_PER_OBJECT_DESCRIPTOR_LAYOUT_BINDING_COUNT;
    perObjectDescriptorSetLayoutInfo.pBindings = STATIC_PIPELINE_PER_OBJECT_DESCRIPTOR_LAYOUT_BINDINGS;
    ASSERT_VK_RESULT_SUCCESS(
        vkCreateDescriptorSetLayout(context->GetLogicalDevice(), &perObjectDescriptorSetLayoutInfo, nullptr, &perObjectDescriptorSetLayout),
        "Failed to create per object descriptor set layout");

    VkDescriptorSetLayout descriptorSetLayouts[] = { uniformDescriptorSetLayout, perObjectDescriptorSetLayout };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.setLayoutCount = 2;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
    ASSERT_VK_RESULT_SUCCESS(
        vkCreatePipelineLayout(context->GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout),
        "Failed to create pipeline layout");

    VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = vertexShader->GetShaderModule();
    vertexShaderStageInfo.pName = SHADER_MAIN_FUNCTION_NAME;

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = fragmentShader->GetShaderModule();
    fragmentShaderStageInfo.pName = SHADER_MAIN_FUNCTION_NAME;

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_COUNT;
    vertexInputInfo.pVertexAttributeDescriptions = VERTEX_INPUT_ATTRIBUTE_DESCRIPTIONS;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = nullptr;
    viewportState.scissorCount = 1;
    viewportState.pScissors = nullptr;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStates;
    dynamicState.dynamicStateCount = 2;
    dynamicState.flags = 0;

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo };
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = context->GetRenderPass();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    ASSERT_VK_RESULT_SUCCESS(
        vkCreateGraphicsPipelines(context->GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline),
        "Failed to create pipeline");
}

void VulkanPipeline::Destroy()
{
    vkDestroyDescriptorSetLayout(context->GetLogicalDevice(), uniformDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(context->GetLogicalDevice(), perObjectDescriptorSetLayout, nullptr);
    vkDestroyPipeline(context->GetLogicalDevice(), pipeline, nullptr);
    vkDestroyPipelineLayout(context->GetLogicalDevice(), pipelineLayout, nullptr);
}

void VulkanPipeline::Recreate()
{
    Destroy();
    Create();
}

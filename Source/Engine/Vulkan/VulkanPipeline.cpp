#include "Engine/Mesh.h"
#include "VulkanCommon.h"
#include "VulkanPipeline.h"

VulkanPipeline::VulkanPipeline(VulkanContext *context, VulkanRenderPass *renderPass)
    : context(context),
      renderPass(renderPass),
      uniformDescriptorSetLayout(),
      imageDescriptorSetLayout(),
      instanceDescriptorSetLayout(),
      pipeline(),
      pipelineLayout()
{
}

VulkanPipeline::~VulkanPipeline()
{
}

void VulkanPipeline::Create(VulkanPipelineConfig config)
{
    CreateDescriptorLayouts();

    VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = config.vertexShader->GetShaderModule();
    vertexShaderStageInfo.pName = SHADER_MAIN_FUNCTION_NAME;

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = config.fragmentShader->GetShaderModule();
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
    rasterizer.cullMode = config.cullMode;
    rasterizer.frontFace = config.frontFace;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = context->GetMSAASampleBits();

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
    pipelineInfo.renderPass = renderPass->GetRenderPass();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    ASSERT_VK_RESULT_SUCCESS(
        vkCreateGraphicsPipelines(context->GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline),
        "Failed to create pipeline");
}

void VulkanPipeline::CreateDescriptorLayouts()
{
    VkDescriptorSetLayoutCreateInfo uniformDescriptorSetLayoutInfo{};
    uniformDescriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    uniformDescriptorSetLayoutInfo.bindingCount = STATIC_PIPELINE_UNIFORM_DESCRIPTOR_LAYOUT_BINDING_COUNT;
    uniformDescriptorSetLayoutInfo.pBindings = STATIC_PIPELINE_UNIFORM_DESCRIPTOR_LAYOUT_BINDINGS;
    ASSERT_VK_RESULT_SUCCESS(
        vkCreateDescriptorSetLayout(context->GetLogicalDevice(), &uniformDescriptorSetLayoutInfo, nullptr, &uniformDescriptorSetLayout),
        "Failed to create uniform descriptor set layout");

    VkDescriptorSetLayoutCreateInfo imageDescriptorSetLayoutInfo{};
    imageDescriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    imageDescriptorSetLayoutInfo.bindingCount = STATIC_PIPELINE_IMAGE_DESCRIPTOR_LAYOUT_BINDING_COUNT;
    imageDescriptorSetLayoutInfo.pBindings = STATIC_PIPELINE_IMAGE_DESCRIPTOR_LAYOUT_BINDINGS;
    ASSERT_VK_RESULT_SUCCESS(
        vkCreateDescriptorSetLayout(context->GetLogicalDevice(), &imageDescriptorSetLayoutInfo, nullptr, &imageDescriptorSetLayout),
        "Failed to create image descriptor set layout");

    VkDescriptorSetLayoutCreateInfo instanceDescriptorSetLayoutInfo{};
    instanceDescriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    instanceDescriptorSetLayoutInfo.bindingCount = STATIC_PIPELINE_INSTANCE_DESCRIPTOR_LAYOUT_BINDING_COUNT;
    instanceDescriptorSetLayoutInfo.pBindings = STATIC_PIPELINE_INSTANCE_DESCRIPTOR_LAYOUT_BINDINGS;
    ASSERT_VK_RESULT_SUCCESS(
        vkCreateDescriptorSetLayout(context->GetLogicalDevice(), &instanceDescriptorSetLayoutInfo, nullptr, &instanceDescriptorSetLayout),
        "Failed to create instance descriptor set layout");

    VkDescriptorSetLayout descriptorSetLayouts[] = { uniformDescriptorSetLayout, imageDescriptorSetLayout, instanceDescriptorSetLayout };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.setLayoutCount = 3;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
    ASSERT_VK_RESULT_SUCCESS(
        vkCreatePipelineLayout(context->GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout),
        "Failed to create pipeline layout");
}

void VulkanPipeline::Destroy()
{
    DestroyDescriptorLayouts();

    VkDevice logicalDevice = context->GetLogicalDevice();
    vkDestroyPipeline(logicalDevice, pipeline, nullptr);
    vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
}

void VulkanPipeline::DestroyDescriptorLayouts()
{
    VkDevice logicalDevice = context->GetLogicalDevice();
    vkDestroyDescriptorSetLayout(logicalDevice, uniformDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(logicalDevice, imageDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(logicalDevice, instanceDescriptorSetLayout, nullptr);
}

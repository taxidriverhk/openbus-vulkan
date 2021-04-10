#include "Engine/Mesh.h"
#include "VulkanCommon.h"
#include "VulkanPipeline.h"

VulkanPipeline::VulkanPipeline(VulkanContext *context, VulkanShader *vertexShader, VulkanShader *fragmentShader)
    : context(context),
      uniformDescriptorSetLayout(),
      imageDescriptorSetLayout(),
      instanceDescriptorSetLayout(),
      fragmentShader(fragmentShader),
      vertexShader(vertexShader),
      renderPass(),
      pipeline(),
      pipelineLayout()
{
}

VulkanPipeline::~VulkanPipeline()
{
}

void VulkanPipeline::Create()
{
    CreateRenderPass();

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
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

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
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    ASSERT_VK_RESULT_SUCCESS(
        vkCreateGraphicsPipelines(context->GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline),
        "Failed to create pipeline");
}

void VulkanPipeline::Destroy()
{
    VkDevice logicalDevice = context->GetLogicalDevice();
    vkDestroyDescriptorSetLayout(logicalDevice, uniformDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(logicalDevice, imageDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(logicalDevice, instanceDescriptorSetLayout, nullptr);
    vkDestroyPipeline(logicalDevice, pipeline, nullptr);
    vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
    vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
}

void VulkanPipeline::Recreate()
{
    Destroy();
    Create();
}

void VulkanPipeline::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = context->GetSwapChainImageFormat();
    colorAttachment.samples = context->GetMSAASampleBits();
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = context->GetDepthImageFormat();
    depthAttachment.samples = context->GetMSAASampleBits();
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = context->GetSwapChainImageFormat();
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment, colorAttachmentResolve };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 3;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    ASSERT_VK_RESULT_SUCCESS(
        vkCreateRenderPass(context->GetLogicalDevice(), &renderPassInfo, nullptr, &renderPass),
        "Failed to create render pass");
}

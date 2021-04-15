#include "Engine/Vulkan/VulkanCommon.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/VulkanPipeline.h"
#include "Engine/Vulkan/VulkanRenderPass.h"
#include "Engine/Vulkan/Buffer/VulkanBuffer.h"
#include "Engine/Vulkan/Image/VulkanImage.h"
#include "VulkanCommand.h"
#include "VulkanCommandManager.h"

VulkanCommandManager::VulkanCommandManager(
    VulkanContext *context,
    VulkanRenderPass *renderPass,
    VulkanDrawingPipelines pipelines)
    : context(context),
      renderPass(renderPass),
      pipelines(pipelines),
      frameBufferSize(0),
      dataUpdated()
{
}

VulkanCommandManager::~VulkanCommandManager()
{
}

void VulkanCommandManager::Create(uint32_t frameBufferSize)
{
    VkCommandPool commandPool = GetOrCreateCommandPool(std::this_thread::get_id());

    this->frameBufferSize = frameBufferSize;
    for (uint32_t i = 0; i < frameBufferSize; i++)
    {
        std::unique_ptr<VulkanCommand> commandBuffer = std::make_unique<VulkanCommand>(context, commandPool);
        commandBuffer->Create();

        commandBuffers.push_back(std::move(commandBuffer));
        dataUpdated.push_back(true);
    }
}

VkCommandPool VulkanCommandManager::GetOrCreateCommandPool(std::thread::id threadId)
{
    if (commandPools.count(threadId) > 0)
    {
        return commandPools[threadId];
    }

    VkCommandPool commandPool;
    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = context->GetGraphicsQueueIndex();
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_VK_RESULT_SUCCESS(
        vkCreateCommandPool(context->GetLogicalDevice(), &commandPoolInfo, nullptr, &commandPool),
        "Failed to create command pool");
    commandPools.insert(std::make_pair(threadId, commandPool));

    return commandPools[threadId];
}

void VulkanCommandManager::Destroy()
{
    for (auto &[threadId, commandPool] : commandPools)
    {
        vkDestroyCommandPool(context->GetLogicalDevice(), commandPool, nullptr);
    }
    commandBuffers.clear();
    commandPools.clear();
}

void VulkanCommandManager::Record(
    uint32_t imageIndex,
    VkFramebuffer framebuffer,
    VulkanBuffer *uniformBuffer,
    VulkanCubeMapBuffer &cubeMapBuffer,
    std::unordered_map<uint32_t, VulkanDrawingCommand> &drawingCommands)
{
    if (!dataUpdated[imageIndex])
    {
        return;
    }

    VkCommandBuffer commandBuffer = BeginCommand(imageIndex, framebuffer);

    VulkanPipeline *staticPipeline = pipelines.staticPipeline;
    VulkanPipeline *cubeMapPipeline = pipelines.cubeMapPipeline;
    for (const auto &[bufferId, drawingCommand] : drawingCommands)
    {
        BindPipeline(commandBuffer, staticPipeline);

        VulkanBuffer *instanceBuffer = drawingCommand.instanceBuffer;
        VulkanBuffer *vertexBuffer = drawingCommand.vertexBuffer;
        VulkanBuffer *indexBuffer = drawingCommand.indexBuffer;
        VulkanImage *imageBuffer = drawingCommand.imageBuffer;

        // Bind vertex buffer
        VkBuffer vertexBuffers[] = { vertexBuffer->GetBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        // Bind index buffer
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        // Bind uniform descriptor set
        uniformBuffer->BindDescriptorSet(commandBuffer, 0, staticPipeline->GetPipelineLayout());
        // Bind image sampler descriptor set
        imageBuffer->BindDescriptorSet(commandBuffer, 1, staticPipeline->GetPipelineLayout());
        // Bind instance descriptor set
        instanceBuffer->BindDescriptorSet(commandBuffer, 2, staticPipeline->GetPipelineLayout());

        uint32_t indexCount = indexBuffer->Size() / sizeof(uint32_t);
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    }

    VulkanBuffer *cubeMapVertexBuffer = cubeMapBuffer.vertexBuffer;
    VulkanBuffer *cubeMapIndexBuffer = cubeMapBuffer.indexBuffer;
    if (cubeMapVertexBuffer->IsLoaded())
    {
        BindPipeline(commandBuffer, cubeMapPipeline);
        // Bind cubemap mesh
        VkDeviceSize offsets[] = { 0 };
        VkBuffer cubeMapVertexBuffers[] = { cubeMapVertexBuffer->GetBuffer() };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, cubeMapVertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, cubeMapIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        // Bind cubemap descriptor set
        cubeMapBuffer.imageBuffer->BindDescriptorSet(commandBuffer, 1, cubeMapPipeline->GetPipelineLayout());
        // Draw the cube map
        uint32_t indexCount = cubeMapIndexBuffer->Size() / sizeof(uint32_t);
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    }

    EndCommand(commandBuffer);

    dataUpdated[imageIndex] = false;
}

VkCommandBuffer VulkanCommandManager::BeginCommand(uint32_t imageIndex, VkFramebuffer frameBuffer)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    VkCommandBuffer commandBuffer = commandBuffers[imageIndex]->GetBuffer();
    ASSERT_VK_RESULT_SUCCESS(
        vkBeginCommandBuffer(commandBuffer, &beginInfo),
        "Failed to begin command buffer");

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(context->GetSwapChainExtent().width);
    viewport.height = static_cast<float>(context->GetSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = context->GetSwapChainExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass->GetRenderPass();
    renderPassBeginInfo.framebuffer = frameBuffer;
    renderPassBeginInfo.renderArea.offset = { 0, 0 };
    renderPassBeginInfo.renderArea.extent = context->GetSwapChainExtent();

    VkClearValue clearValues[2];
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    return commandBuffer;
}

void VulkanCommandManager::BindPipeline(VkCommandBuffer commandBuffer, VulkanPipeline *pipeline)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());
}


void VulkanCommandManager::EndCommand(VkCommandBuffer commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
    ASSERT_VK_RESULT_SUCCESS(vkEndCommandBuffer(commandBuffer), "Failed to end the command buffer");
}

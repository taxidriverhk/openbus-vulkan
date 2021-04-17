#include <future>

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

        primaryCommandBuffers.push_back(std::move(commandBuffer));
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
    primaryCommandBuffers.clear();
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

    VkCommandBuffer primaryCommandBuffer = BeginPrimaryCommand(imageIndex, framebuffer);

    std::mutex secondaryCommandMutex;
    std::vector<VkCommandBuffer> secondaryCommandBuffers;

    std::future<void> staticCommandFuture = std::async(std::launch::async, [&]()
        {
            VkCommandBuffer secondaryCommandBuffer = BeginSecondaryCommand(imageIndex, framebuffer);

            VulkanPipeline *staticPipeline = pipelines.staticPipeline;
            for (const auto &[bufferId, drawingCommand] : drawingCommands)
            {
                BindPipeline(secondaryCommandBuffer, staticPipeline);

                VulkanBuffer *instanceBuffer = drawingCommand.instanceBuffer;
                VulkanBuffer *vertexBuffer = drawingCommand.vertexBuffer;
                VulkanBuffer *indexBuffer = drawingCommand.indexBuffer;
                VulkanImage *imageBuffer = drawingCommand.imageBuffer;

                // Bind vertex buffer
                VkBuffer vertexBuffers[] = { vertexBuffer->GetBuffer() };
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(secondaryCommandBuffer, 0, 1, vertexBuffers, offsets);
                // Bind index buffer
                vkCmdBindIndexBuffer(secondaryCommandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
                // Bind uniform descriptor set
                uniformBuffer->BindDescriptorSet(secondaryCommandBuffer, 0, staticPipeline->GetPipelineLayout());
                // Bind image sampler descriptor set
                imageBuffer->BindDescriptorSet(secondaryCommandBuffer, 1, staticPipeline->GetPipelineLayout());
                // Bind instance descriptor set
                instanceBuffer->BindDescriptorSet(secondaryCommandBuffer, 2, staticPipeline->GetPipelineLayout());

                uint32_t indexCount = indexBuffer->Size() / sizeof(uint32_t);
                vkCmdDrawIndexed(secondaryCommandBuffer, indexCount, 1, 0, 0, 0);
            }

            secondaryCommandMutex.lock();
            secondaryCommandBuffers.push_back(secondaryCommandBuffer);
            secondaryCommandMutex.unlock();

            EndCommand(secondaryCommandBuffer);
        });

    std::future<void> cubeMapFuture = std::async(std::launch::async, [&]()
        {
            VkCommandBuffer secondaryCommandBuffer = BeginSecondaryCommand(imageIndex, framebuffer);

            VulkanPipeline *staticPipeline = pipelines.staticPipeline;
            VulkanPipeline *cubeMapPipeline = pipelines.cubeMapPipeline;

            VulkanBuffer *cubeMapVertexBuffer = cubeMapBuffer.vertexBuffer;
            VulkanBuffer *cubeMapIndexBuffer = cubeMapBuffer.indexBuffer;
            if (cubeMapVertexBuffer->IsLoaded())
            {
                BindPipeline(secondaryCommandBuffer, cubeMapPipeline);
                // Bind cubemap mesh
                VkDeviceSize offsets[] = { 0 };
                VkBuffer cubeMapVertexBuffers[] = { cubeMapVertexBuffer->GetBuffer() };
                vkCmdBindVertexBuffers(secondaryCommandBuffer, 0, 1, cubeMapVertexBuffers, offsets);
                vkCmdBindIndexBuffer(secondaryCommandBuffer, cubeMapIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
                // Bind uniform descriptor set
                uniformBuffer->BindDescriptorSet(secondaryCommandBuffer, 0, staticPipeline->GetPipelineLayout());
                // Bind cubemap descriptor set
                cubeMapBuffer.imageBuffer->BindDescriptorSet(secondaryCommandBuffer, 1, cubeMapPipeline->GetPipelineLayout());
                // Draw the cube map
                uint32_t indexCount = cubeMapIndexBuffer->Size() / sizeof(uint32_t);
                vkCmdDrawIndexed(secondaryCommandBuffer, indexCount, 1, 0, 0, 0);
            }

            secondaryCommandMutex.lock();
            secondaryCommandBuffers.push_back(secondaryCommandBuffer);
            secondaryCommandMutex.unlock();

            EndCommand(secondaryCommandBuffer);
        });

    staticCommandFuture.get();
    cubeMapFuture.get();

    vkCmdExecuteCommands(
        primaryCommandBuffer,
        static_cast<uint32_t>(secondaryCommandBuffers.size()),
        secondaryCommandBuffers.data());

    vkCmdEndRenderPass(primaryCommandBuffer);
    EndCommand(primaryCommandBuffer);

    dataUpdated[imageIndex] = false;
}

void VulkanCommandManager::TriggerUpdate(uint32_t imageIndex)
{
    dataUpdated[imageIndex] = true;
}

VkCommandBuffer VulkanCommandManager::BeginPrimaryCommand(uint32_t imageIndex, VkFramebuffer frameBuffer)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

    VkCommandBuffer commandBuffer = primaryCommandBuffers[imageIndex]->GetBuffer();
    ASSERT_VK_RESULT_SUCCESS(
        vkBeginCommandBuffer(commandBuffer, &beginInfo),
        "Failed to begin command buffer");

    SetViewPortAndScissor(commandBuffer);

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

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    return commandBuffer;
}

VkCommandBuffer VulkanCommandManager::BeginSecondaryCommand(uint32_t imageIndex, VkFramebuffer frameBuffer)
{
    VulkanCommand *secondaryCommandBuffer = RequestSecondaryCommandBuffer(imageIndex);
    VkCommandBuffer commandBuffer = secondaryCommandBuffer->GetBuffer();

    VkCommandBufferInheritanceInfo inheritanceInfo{};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.framebuffer = frameBuffer;
    inheritanceInfo.renderPass = renderPass->GetRenderPass();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    ASSERT_VK_RESULT_SUCCESS(
        vkBeginCommandBuffer(commandBuffer, &beginInfo),
        "Failed to begin command buffer");

    SetViewPortAndScissor(commandBuffer);

    return commandBuffer;
}

void VulkanCommandManager::BindPipeline(VkCommandBuffer commandBuffer, VulkanPipeline *pipeline)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());
}


void VulkanCommandManager::EndCommand(VkCommandBuffer commandBuffer)
{
    ASSERT_VK_RESULT_SUCCESS(vkEndCommandBuffer(commandBuffer), "Failed to end the command buffer");
}

VulkanCommand * VulkanCommandManager::RequestSecondaryCommandBuffer(uint32_t imageIndex)
{
    std::thread::id threadId = std::this_thread::get_id();
    VkCommandPool commandPool = GetOrCreateCommandPool(threadId);

    if (secondaryCommandBufferCache.count(threadId) > 0)
    {
        SecondaryCommandBufferCache &cache = secondaryCommandBufferCache[threadId];
        if (cache.buffersInUse < cache.buffers.size())
        {
            cache.buffersInUse++;
            return cache.buffers[cache.buffersInUse].get();
        }
        else
        {
            std::unique_ptr<VulkanCommand> commandBuffer = std::make_unique<VulkanCommand>(context, commandPool);
            commandBuffer->Create(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

            cache.buffers.push_back(std::move(commandBuffer));
            cache.buffersInUse++;
            return cache.buffers.back().get();
        }
    }
    else
    {
        std::unique_ptr<VulkanCommand> commandBuffer = std::make_unique<VulkanCommand>(context, commandPool);
        commandBuffer->Create(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

        secondaryCommandBufferCache[threadId] = SecondaryCommandBufferCache{};
        secondaryCommandBufferCache[threadId].buffers.push_back(std::move(commandBuffer));
        secondaryCommandBufferCache[threadId].buffersInUse = 1;

        return secondaryCommandBufferCache[threadId].buffers.back().get();
    }
}

void VulkanCommandManager::ResetSecondaryCommandBuffers()
{
    for (auto &[threadId, cache] : secondaryCommandBufferCache)
    {
        cache.buffersInUse = 0;
        for (auto &buffer : cache.buffers)
        {
            buffer->Reset();
        }
    }
}

void VulkanCommandManager::SetViewPortAndScissor(VkCommandBuffer commandBuffer)
{
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
}

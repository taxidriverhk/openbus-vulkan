#include <future>

#include "Common/Logger.h"
#include "Engine/Vulkan/VulkanCommon.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/VulkanRenderPass.h"
#include "Engine/Vulkan/Buffer/VulkanBuffer.h"
#include "Engine/Vulkan/Image/VulkanImage.h"
#include "Engine/Vulkan/Pipeline/VulkanPipeline.h"
#include "VulkanCommandPool.h"
#include "VulkanCommandManager.h"

VulkanCommandManager::VulkanCommandManager(
    VulkanContext *context,
    VulkanCommandPool *commandPool,
    VulkanRenderPass *renderPass,
    VulkanDrawingPipelines pipelines)
    : context(context),
      commandPool(commandPool),
      renderPass(renderPass),
      pipelines(pipelines),
      frameBufferSize(0)
{
}

VulkanCommandManager::~VulkanCommandManager()
{
}

void VulkanCommandManager::Create(uint32_t frameBufferSize)
{
    VkCommandPool commandPoolToUse = commandPool->GetOrCreateCommandPool(std::this_thread::get_id());

    this->frameBufferSize = frameBufferSize;
    for (uint32_t i = 0; i < frameBufferSize; i++)
    {
        std::unique_ptr<VulkanCommand> commandBuffer = std::make_unique<VulkanCommand>(context, commandPoolToUse);
        commandBuffer->Create();

        primaryCommandBuffers.push_back(std::move(commandBuffer));
    }
}

void VulkanCommandManager::Destroy()
{
    for (auto &[threadId, secondaryCommandBuffer] : secondaryCommandBuffers)
    {
        for (auto &commandBufferForFrame : secondaryCommandBuffer)
        {
            for (auto &commandBuffer : commandBufferForFrame.commandBuffers)
            {
                commandBuffer->Reset();
            }
        }
    }

    secondaryCommandBuffers.clear();
    primaryCommandBuffers.clear();
}

void VulkanCommandManager::Record(
    uint32_t imageIndex,
    VkFramebuffer framebuffer,
    VulkanPushConstants pushConstants,
    VulkanDrawingBuffer drawingBuffer)
{
    VkCommandBuffer primaryCommandBuffer = BeginPrimaryCommand(imageIndex, framebuffer);

    VulkanBuffer *uniformBuffer = drawingBuffer.uniformBuffer;
    VulkanBuffer *screenBuffer = drawingBuffer.screenBuffer;

    VulkanMeshPushConstant &meshPushConstant = pushConstants.meshPushConstant;

    std::mutex secondaryCommandMutex;
    std::vector<VkCommandBuffer> secondaryCommandBuffers;

    std::future<void> staticCommandFuture = std::async(std::launch::async, [&]()
        {
            secondaryCommandMutex.lock();
            VkCommandBuffer secondaryCommandBuffer = BeginSecondaryCommand(imageIndex, framebuffer);
            secondaryCommandMutex.unlock();

            VulkanPipeline *staticPipeline = pipelines.staticPipeline;
            BindPipeline(secondaryCommandBuffer, staticPipeline);

            PushConstant(
                secondaryCommandBuffer,
                VK_SHADER_STAGE_VERTEX_BIT,
                staticPipeline,
                &meshPushConstant,
                sizeof(VulkanMeshPushConstant));

            for (const auto &entityBuffer : drawingBuffer.entityBuffers)
            {
                VulkanBuffer *instanceBuffer = entityBuffer.instanceBuffers[imageIndex];
                VulkanBuffer *vertexBuffer = entityBuffer.vertexBuffer;
                VulkanBuffer *indexBuffer = entityBuffer.indexBuffer;
                VulkanImage *imageBuffer = entityBuffer.imageBuffer;

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

    std::future<void> cubeMapCommandFuture = std::async(std::launch::async, [&]()
        {
            secondaryCommandMutex.lock();
            VkCommandBuffer secondaryCommandBuffer = BeginSecondaryCommand(imageIndex, framebuffer);
            secondaryCommandMutex.unlock();

            VulkanPipeline *cubeMapPipeline = pipelines.cubeMapPipeline;

            VulkanCubeMapBuffer cubeMapBuffer = drawingBuffer.cubeMapBuffer;
            VulkanBuffer *cubeMapVertexBuffer = cubeMapBuffer.vertexBuffer;
            VulkanBuffer *cubeMapIndexBuffer = cubeMapBuffer.indexBuffer;
            if (cubeMapVertexBuffer != nullptr && cubeMapVertexBuffer->IsLoaded())
            {
                BindPipeline(secondaryCommandBuffer, cubeMapPipeline);
                // Bind cubemap mesh
                VkDeviceSize offsets[] = { 0 };
                VkBuffer cubeMapVertexBuffers[] = { cubeMapVertexBuffer->GetBuffer() };
                vkCmdBindVertexBuffers(secondaryCommandBuffer, 0, 1, cubeMapVertexBuffers, offsets);
                vkCmdBindIndexBuffer(secondaryCommandBuffer, cubeMapIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
                // Bind uniform descriptor set
                uniformBuffer->BindDescriptorSet(secondaryCommandBuffer, 0, cubeMapPipeline->GetPipelineLayout());
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

    std::future<void> terrainCommandFuture = std::async(std::launch::async, [&]()
        {
            secondaryCommandMutex.lock();
            VkCommandBuffer secondaryCommandBuffer = BeginSecondaryCommand(imageIndex, framebuffer);
            secondaryCommandMutex.unlock();

            VulkanPipeline *terrainPipeline = pipelines.terrainPipeline;
            BindPipeline(secondaryCommandBuffer, terrainPipeline);

            PushConstant(
                secondaryCommandBuffer,
                VK_SHADER_STAGE_VERTEX_BIT,
                terrainPipeline,
                &meshPushConstant,
                sizeof(VulkanMeshPushConstant));

            for (const auto &terrainBuffer : drawingBuffer.terrainBuffers)
            {
                VulkanBuffer *vertexBuffer = terrainBuffer.vertexBuffer;
                VulkanBuffer *indexBuffer = terrainBuffer.indexBuffer;
                VulkanImage *imageBuffer = terrainBuffer.imageBuffer;

                // Bind vertex buffer
                VkBuffer vertexBuffers[] = { vertexBuffer->GetBuffer() };
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(secondaryCommandBuffer, 0, 1, vertexBuffers, offsets);
                // Bind index buffer
                vkCmdBindIndexBuffer(secondaryCommandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
                // Bind uniform descriptor set
                uniformBuffer->BindDescriptorSet(secondaryCommandBuffer, 0, terrainPipeline->GetPipelineLayout());
                // Bind image sampler descriptor set
                imageBuffer->BindDescriptorSet(secondaryCommandBuffer, 1, terrainPipeline->GetPipelineLayout());

                uint32_t indexCount = indexBuffer->Size() / sizeof(uint32_t);
                vkCmdDrawIndexed(secondaryCommandBuffer, indexCount, 1, 0, 0, 0);
            }

            secondaryCommandMutex.lock();
            secondaryCommandBuffers.push_back(secondaryCommandBuffer);
            secondaryCommandMutex.unlock();

            EndCommand(secondaryCommandBuffer);
        });

    terrainCommandFuture.get();
    staticCommandFuture.get();
    cubeMapCommandFuture.get();

    // Screen objects must appear on top of everthing else
    // Otherwise, the screen object could be blended by something else
    std::future<void> screenCommandFuture = std::async(std::launch::async, [&]()
        {
            secondaryCommandMutex.lock();
            VkCommandBuffer secondaryCommandBuffer = BeginSecondaryCommand(imageIndex, framebuffer);
            secondaryCommandMutex.unlock();

            VulkanPipeline *screenPipeline = pipelines.screenPipeline;
            BindPipeline(secondaryCommandBuffer, screenPipeline);

            for (const auto &screenObjectBuffer : drawingBuffer.screenObjectBuffers)
            {
                VulkanBuffer *vertexBuffer = screenObjectBuffer.vertexBuffer;
                VulkanImage *imageBuffer = screenObjectBuffer.imageBuffer;

                // Bind vertex buffer
                VkBuffer vertexBuffers[] = { vertexBuffer->GetBuffer() };
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(secondaryCommandBuffer, 0, 1, vertexBuffers, offsets);
                // Bind uniform descriptor set
                screenBuffer->BindDescriptorSet(secondaryCommandBuffer, 0, screenPipeline->GetPipelineLayout());
                // Bind image sampler descriptor set
                imageBuffer->BindDescriptorSet(secondaryCommandBuffer, 1, screenPipeline->GetPipelineLayout());

                uint32_t vertexCount = vertexBuffer->Size() / sizeof(ScreenObjectVertex);
                vkCmdDraw(secondaryCommandBuffer, vertexCount, 1, 0, 0);
            }

            secondaryCommandMutex.lock();
            secondaryCommandBuffers.push_back(secondaryCommandBuffer);
            secondaryCommandMutex.unlock();

            EndCommand(secondaryCommandBuffer);
        });

    screenCommandFuture.get();

    vkCmdExecuteCommands(
        primaryCommandBuffer,
        static_cast<uint32_t>(secondaryCommandBuffers.size()),
        secondaryCommandBuffers.data());

    vkCmdEndRenderPass(primaryCommandBuffer);
    EndCommand(primaryCommandBuffer);
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

void VulkanCommandManager::PushConstant(
    VkCommandBuffer commandBuffer,
    VkShaderStageFlags stage,
    VulkanPipeline *pipeline,
    void *data,
    uint32_t size)
{
    vkCmdPushConstants(
        commandBuffer,
        pipeline->GetPipelineLayout(),
        stage,
        0,
        size,
        data);
}

VulkanCommand * VulkanCommandManager::RequestSecondaryCommandBuffer(uint32_t imageIndex)
{
    // This method is not thread-safe, so need to have a mutx to ensure
    // that only one thread is calling this function at a time
    std::thread::id threadId = std::this_thread::get_id();
    VkCommandPool commandPoolToUse = commandPool->GetOrCreateCommandPool(threadId);

    if (secondaryCommandBuffers.count(threadId) == 0)
    {
        secondaryCommandBuffers[threadId].resize(frameBufferSize);
        secondaryCommandBuffers[threadId][imageIndex].activeBuffersInUse = 0;
    }

    // This is obviously not be the best solution
    // But if the same thread ID is assigned to two different secondary command buffers at the same time
    // Then one of them must create a new command buffer, to avoid conflict with the other one
    auto &secondaryCommandBuffer = secondaryCommandBuffers[threadId][imageIndex];
    if (secondaryCommandBuffer.activeBuffersInUse < secondaryCommandBuffer.commandBuffers.size())
    {
        Logger::Log(LogLevel::Debug, "Secondary command buffer is already created and can be reused");
        int currentIndex = secondaryCommandBuffer.activeBuffersInUse;
        secondaryCommandBuffer.activeBuffersInUse++;
        return secondaryCommandBuffer.commandBuffers[currentIndex].get();
    }
    // Allocate extra command buffer for the thread in case all existing command buffers are in use
    else
    {
        Logger::Log(LogLevel::Debug, "Secondary command buffer is either not created or already in use");
        std::unique_ptr<VulkanCommand> commandBuffer = std::make_unique<VulkanCommand>(context, commandPoolToUse);
        commandBuffer->Create(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

        secondaryCommandBuffer.commandBuffers.push_back(std::move(commandBuffer));
        secondaryCommandBuffer.activeBuffersInUse++;

        return secondaryCommandBuffer.commandBuffers.back().get();
    }
}

void VulkanCommandManager::Reset(uint32_t previousImageIndex)
{
    // Allows threads to reuse the command buffer that were previously created
    for (auto &[threadId, secondaryCommandBuffer] : secondaryCommandBuffers)
    {
        secondaryCommandBuffer[previousImageIndex].activeBuffersInUse = 0;
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

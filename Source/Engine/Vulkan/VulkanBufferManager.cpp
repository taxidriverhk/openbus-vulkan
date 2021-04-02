#include "VulkanBufferManager.h"
#include "Buffer/VulkanIndexBuffer.h"
#include "Buffer/VulkanVertexBuffer.h"

VulkanBufferManager::VulkanBufferManager(VulkanContext *context, VulkanPipeline *pipeline)
    : context(context),
      pipeline(pipeline),
      commandPool(),
      descriptorPool(),
      currentInFlightFrame(0),
      indexBuffers(),
      vertexBuffers(),
      uniformBufferUpdated(true),
      uniformBuffers(),
      uniformBufferDescriptorSets(),
      uniformBufferInput()
{
}

VulkanBufferManager::~VulkanBufferManager()
{
}

void VulkanBufferManager::Create()
{
    CreateFrameBuffers();

    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = context->GetGraphicsQueueIndex();
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(context->GetLogicalDevice(), &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool");
    }

    CreateCommandBuffers();
    CreateSynchronizationObjects();
    CreateDescriptorPool();
    CreateUniformBuffers();
}

void VulkanBufferManager::Destroy()
{
    DestroyUniformBuffers();

    VkDevice logicalDevice = context->GetLogicalDevice();
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(logicalDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(logicalDevice, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(logicalDevice, inFlightFences[i], nullptr);
    }

    DestroyCommandBuffers();
    vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

    DestroyFrameBuffers();
}

void VulkanBufferManager::Draw(uint32_t &imageIndex)
{
    BeginFrame(imageIndex);
    Submit(imageIndex);
    EndFrame(imageIndex);
}

void VulkanBufferManager::BeginFrame(uint32_t &imageIndex)
{
    VkDevice logicalDevice = context->GetLogicalDevice();
    VkSwapchainKHR swapChain = context->GetSwapChain();

    vkWaitForFences(logicalDevice, 1, &inFlightFences[currentInFlightFrame], VK_TRUE, UINT64_MAX);

    VkResult acquireImageResult = vkAcquireNextImageKHR(
        logicalDevice,
        swapChain,
        UINT64_MAX,
        imageAvailableSemaphores[currentInFlightFrame],
        VK_NULL_HANDLE,
        &imageIndex);
    if (acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChainAndBuffers();
        return;
    }

    if (uniformBufferUpdated)
    {
        uniformBuffers[imageIndex]->UpdateBufferData(&uniformBufferInput, sizeof(VulkanUniformBufferInput));
        uniformBufferUpdated = false;
    }

    // The acquire image is still in use
    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(logicalDevice, 1, &imagesInFlight[currentInFlightFrame], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentInFlightFrame];
}

void VulkanBufferManager::EndFrame(uint32_t &imageIndex)
{
    VkSwapchainKHR swapChain = context->GetSwapChain();
    VkQueue presentQueue = context->GetPresentQueue();

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentInFlightFrame] };
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    VkResult presentImageResult = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (presentImageResult == VK_ERROR_OUT_OF_DATE_KHR
        || presentImageResult == VK_SUBOPTIMAL_KHR
        || context->GetScreen()->IsResized())
    {
        context->GetScreen()->ResetResizeState();
        RecreateSwapChainAndBuffers();
    }

    currentInFlightFrame = (currentInFlightFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanBufferManager::LoadIntoBuffer(uint32_t bufferId, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices)
{
    std::unique_ptr<VulkanBuffer> vertexBuffer = std::make_unique<VulkanVertexBuffer>(
        context, commandPool, vertices);
    vertexBuffer->Load();
    vertexBuffers.insert(std::make_pair(bufferId, std::move(vertexBuffer)));

    std::unique_ptr<VulkanBuffer> indexBuffer = std::make_unique<VulkanIndexBuffer>(
        context, commandPool, indices);
    indexBuffer->Load();
    indexBuffers.insert(std::make_pair(bufferId, std::move(indexBuffer)));

    RecordCommandBuffers();
}

void VulkanBufferManager::Submit(uint32_t &imageIndex)
{
    VkDevice logicalDevice = context->GetLogicalDevice();
    VkQueue graphicsQueue = context->GetGraphicsQueue();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore semaphoresToWaitFor[] = { imageAvailableSemaphores[currentInFlightFrame] };
    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentInFlightFrame] };
    VkPipelineStageFlags stagesToWaitFor[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = semaphoresToWaitFor;
    submitInfo.pWaitDstStageMask = stagesToWaitFor;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(logicalDevice, 1, &inFlightFences[currentInFlightFrame]);

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentInFlightFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit the command for drawing the buffer");
    }
}

void VulkanBufferManager::UnloadBuffer(uint32_t bufferId)
{
    std::unique_ptr<VulkanBuffer> &vertexBuffer = vertexBuffers[bufferId];
    vertexBuffer->Unload();
    vertexBuffers.erase(bufferId);

    std::unique_ptr<VulkanBuffer> &indexBuffer = indexBuffers[bufferId];
    indexBuffer->Unload();
    indexBuffers.erase(bufferId);
}

void VulkanBufferManager::UpdateUniformBuffer(VulkanUniformBufferInput input)
{
    uniformBufferInput = input;
    uniformBufferUpdated = true;
}

void VulkanBufferManager::CreateCommandBuffers()
{
    commandBuffers.resize(frameBuffers.size());
    VkCommandBufferAllocateInfo commandBufferInfo{};
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferInfo.commandPool = commandPool;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
    if (vkAllocateCommandBuffers(context->GetLogicalDevice(), &commandBufferInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command buffers");
    }
}

void VulkanBufferManager::CreateDescriptorPool()
{
    uint32_t frameBufferSize = frameBuffers.size();

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(frameBufferSize);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(frameBufferSize);

    if (vkCreateDescriptorPool(context->GetLogicalDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool");
    }
}

void VulkanBufferManager::CreateFrameBuffers()
{
    VkRenderPass renderPass = context->GetRenderPass();
    VkExtent2D swapChainExtent = context->GetSwapChainExtent();
    std::vector<VkImageView> swapChainImageViews = context->GetSwapChainImageViews();
    for (VkImageView swapChainImageView : swapChainImageViews)
    {
        VkFramebuffer frameBuffer;
        VkImageView attachments[] = { swapChainImageView };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;
        if (vkCreateFramebuffer(context->GetLogicalDevice(), &framebufferInfo, nullptr, &frameBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create frame buffer");
        }

        frameBuffers.push_back(frameBuffer);
    }
}

void VulkanBufferManager::CreateSynchronizationObjects()
{
    std::vector<VkImage> swapChainImages = context->GetSwapChainImages();
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkDevice logicalDevice = context->GetLogicalDevice();
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS
            || vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS
            || vkCreateFence(logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create synchronization objects for a frame");
        }
    }
}

void VulkanBufferManager::CreateUniformBuffers()
{
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts(frameBuffers.size(), pipeline->GetUniformBufferDescriptorSetLayout());
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(frameBuffers.size());
    descriptorSetAllocInfo.pSetLayouts = descriptorSetLayouts.data();

    uniformBufferDescriptorSets.resize(frameBuffers.size());
    if (vkAllocateDescriptorSets(context->GetLogicalDevice(), &descriptorSetAllocInfo, uniformBufferDescriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets for uniform buffer");
    }

    for (uint32_t i = 0; i < frameBuffers.size(); i++)
    {
        std::unique_ptr<VulkanBuffer> uniformBuffer = std::make_unique<VulkanUniformBuffer>(context, commandPool);
        uniformBuffer->Load();
        uniformBuffers.push_back(std::move(uniformBuffer));

        VkDescriptorBufferInfo uniformBufferInfo{};
        uniformBufferInfo.buffer = uniformBuffers[i]->GetBuffer();
        uniformBufferInfo.offset = 0;
        uniformBufferInfo.range = sizeof(VulkanUniformBufferInput);

        VkWriteDescriptorSet uniformDescriptorWrite{};
        uniformDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uniformDescriptorWrite.dstSet = uniformBufferDescriptorSets[i];
        uniformDescriptorWrite.dstBinding = 0;
        uniformDescriptorWrite.dstArrayElement = 0;
        uniformDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformDescriptorWrite.descriptorCount = 1;
        uniformDescriptorWrite.pBufferInfo = &uniformBufferInfo;

        vkUpdateDescriptorSets(context->GetLogicalDevice(), 1, &uniformDescriptorWrite, 0, nullptr);
    }
}

void VulkanBufferManager::DestroyCommandBuffers()
{
    vkFreeCommandBuffers(
        context->GetLogicalDevice(),
        commandPool,
        static_cast<uint32_t>(commandBuffers.size()),
        commandBuffers.data());
    commandBuffers.clear();
}

void VulkanBufferManager::DestroyFrameBuffers()
{
    VkDevice logicalDevice = context->GetLogicalDevice();
    for (VkFramebuffer frameBuffer : frameBuffers)
    {
        vkDestroyFramebuffer(logicalDevice, frameBuffer, nullptr);
    }
    frameBuffers.clear();
}

void VulkanBufferManager::DestroyUniformBuffers()
{
    vkDestroyDescriptorPool(context->GetLogicalDevice(), descriptorPool, nullptr);
    for (std::unique_ptr<VulkanBuffer> &uniformBuffer : uniformBuffers)
    {
        uniformBuffer->Unload();
    }
    uniformBuffers.clear();
}

void VulkanBufferManager::RecordCommandBuffers()
{
    for (uint32_t i = 0; i < commandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to begin command buffer");
        }

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = context->GetRenderPass();
        renderPassBeginInfo.framebuffer = frameBuffers[i];
        renderPassBeginInfo.renderArea.offset = { 0, 0 };
        renderPassBeginInfo.renderArea.extent = context->GetSwapChainExtent();

        // TODO: add depth stencil value as well
        VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(context->GetSwapChainExtent().width);
        viewport.height = static_cast<float>(context->GetSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = context->GetSwapChainExtent();
        vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());

        for (const auto &vertexBufferEntry : vertexBuffers)
        {
            uint32_t bufferId = vertexBufferEntry.first;
            VulkanBuffer *vertexBuffer = vertexBufferEntry.second.get();
            VulkanBuffer *indexBuffer = indexBuffers[bufferId].get();

            // Bind vertex buffer
            VkBuffer vertexBuffers[] = { vertexBuffer->GetBuffer() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
            // Bind index buffer
            vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
            // Bind uniform buffer
            vkCmdBindDescriptorSets(
                commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipelineLayout(), 0, 1, &uniformBufferDescriptorSets[i], 0, nullptr);

            vkCmdDrawIndexed(commandBuffers[i], indexBuffer->Size(), 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(commandBuffers[i]);

        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to record the command bufferr");
        }
    }
}

void VulkanBufferManager::RecreateSwapChainAndBuffers()
{
    Screen *screen = context->GetScreen();
    int newWidth = screen->GetWidth(),
        newHeight = screen->GetHeight();
    // Do nothing when the window is minimized
    // TODO: this loop may block the entire game,
    // need to have a fix (ex. auto-pause the game)
    while (newWidth == 0 || newHeight == 0)
    {
        newWidth = screen->GetWidth();
        newHeight = screen->GetHeight();
        SDL_WaitEvent(nullptr);
    }
    context->WaitIdle();

    DestroyCommandBuffers();
    DestroyFrameBuffers();

    context->RecreateSwapChain();

    CreateFrameBuffers();
    CreateCommandBuffers();

    // Need to re-record the command buffers
    // since the viewport has become different
    RecordCommandBuffers();
}

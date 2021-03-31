#include "VulkanBufferManager.h"

uint32_t VulkanBufferManager::MAX_FRAMES_IN_FLIGHT = 2;

VulkanBufferManager::VulkanBufferManager(VulkanContext *context, VulkanPipeline *pipeline)
    : context(context),
      pipeline(pipeline),
      commandPool(),
      currentInFlightFrame(0)
{
}

VulkanBufferManager::~VulkanBufferManager()
{
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

    // The acquire image is still in use
    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(logicalDevice, 1, &imagesInFlight[currentInFlightFrame], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentInFlightFrame];
}

void VulkanBufferManager::Create()
{
    CreateFrameBuffers();

    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = context->GetGraphicsQueueIndex();
    if (vkCreateCommandPool(context->GetLogicalDevice(), &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool");
    }

    CreateCommandBuffers();
    CreateSynchronizationObjects();
}

void VulkanBufferManager::Destroy()
{
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

    for (uint32_t i = 0; i < commandBuffers.size(); i++)
    {
        RecordCommandBuffer(i);
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

void VulkanBufferManager::RecordCommandBuffer(uint32_t index)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffers[index], &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin command buffer");
    }

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = context->GetRenderPass();
    renderPassBeginInfo.framebuffer = frameBuffers[index];
    renderPassBeginInfo.renderArea.offset = { 0, 0 };
    renderPassBeginInfo.renderArea.extent = context->GetSwapChainExtent();

    // TODO: add depth stencil value as well
    VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffers[index], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(context->GetSwapChainExtent().width);
    viewport.height = static_cast<float>(context->GetSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = context->GetSwapChainExtent();

    vkCmdSetViewport(commandBuffers[index], 0, 1, &viewport);
    vkCmdSetScissor(commandBuffers[index], 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());
    vkCmdDraw(commandBuffers[index], 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffers[index]);

    if (vkEndCommandBuffer(commandBuffers[index]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to record the command bufferr");
    }
}

void VulkanBufferManager::RecreateSwapChainAndBuffers()
{
    // TODO: implement me, currently minimizing the window will cause issue
    Screen *screen = context->GetScreen();
    int newWidth = screen->GetWidth(),
        newHeight = screen->GetHeight();
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
    pipeline->Recreate();

    CreateFrameBuffers();
    CreateCommandBuffers();
}

#define VMA_IMPLEMENTATION

#include "Engine/Vulkan/VulkanCommon.h"
#include "Engine/Vulkan/Command/VulkanDefaultRenderCommand.h"
#include "VulkanBufferManager.h"

VulkanBufferManager::VulkanBufferManager(VulkanContext *context, VulkanPipeline *pipeline)
    : context(context),
      pipeline(pipeline),
      vmaAllocator(),
      imageVmaAllocator(),
      commandBuffers(),
      commandPool(),
      descriptorPool(),
      currentInFlightFrame(0),
      indexBuffers(),
      vertexBuffers(),
      uniformBufferUpdated(true),
      uniformBuffers(),
      uniformBufferInput(),
      generator(),
      distribution(1, MAX_VERTEX_BUFFERS)
{
}

VulkanBufferManager::~VulkanBufferManager()
{
}

void VulkanBufferManager::Create()
{
    CreateFrameBuffers();

    CreateDescriptorPool();

    CreateMemoryAllocator();

    CreateCommandPool();
    CreateUniformBuffers();
    CreateCommandBuffers();

    CreateSynchronizationObjects();
}

void VulkanBufferManager::Destroy()
{
    DestroyUniformBuffers();

    // Destroying the descriptor pool will automatically free the descriptor sets
    // created using the pool, so no need to explicitly free each descriptor set
    vkDestroyDescriptorPool(context->GetLogicalDevice(), descriptorPool, nullptr);

    VkDevice logicalDevice = context->GetLogicalDevice();
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(logicalDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(logicalDevice, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(logicalDevice, inFlightFences[i], nullptr);
    }

    DestroyCommandBuffers();
    vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

    vmaDestroyAllocator(vmaAllocator);
    vmaDestroyAllocator(imageVmaAllocator);

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

    ASSERT_VK_RESULT_SUCCESS(
        vkWaitForFences(logicalDevice, 1, &inFlightFences[currentInFlightFrame], VK_TRUE, UINT64_MAX),
        "Failed to wait for fences");

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
        uniformBuffers[imageIndex]->Update(&uniformBufferInput, sizeof(VulkanUniformBufferInput));
        uniformBufferUpdated = false;
    }

    // The acquire image is still in use
    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        ASSERT_VK_RESULT_SUCCESS(
            vkWaitForFences(logicalDevice, 1, &imagesInFlight[currentInFlightFrame], VK_TRUE, UINT64_MAX),
            "Failed to wait for fences");
    }
    imagesInFlight[imageIndex] = inFlightFences[currentInFlightFrame];

    // Recording should happen only if the images are changed
    commandBuffers[imageIndex]->Record(frameBuffers[imageIndex]);
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

uint32_t VulkanBufferManager::LoadIntoBuffer(
    uint32_t meshId,
    uint32_t imageId,
    VulkanInstanceBufferInput &instanceBufferInput,
    std::vector<Vertex> &vertices,
    std::vector<uint32_t> &indices,
    Material *material)
{
    uint32_t bufferId = GenerateBufferId();

    if (vertexBuffers.count(meshId) == 0)
    {
        std::shared_ptr<VulkanBuffer> vertexBuffer = std::make_shared<VulkanBuffer>(
            context, commandPool, vmaAllocator);
        vertexBuffer->Load(
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertices.data(),
            static_cast<uint32_t>(sizeof(Vertex) * vertices.size()));
        vertexBuffers.insert(std::make_pair(meshId, std::move(vertexBuffer)));

        std::shared_ptr<VulkanBuffer> indexBuffer = std::make_shared<VulkanBuffer>(
            context, commandPool, vmaAllocator);
        indexBuffer->Load(
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            indices.data(),
            static_cast<uint32_t>(sizeof(uint32_t) * indices.size()));
        indexBuffers.insert(std::make_pair(meshId, std::move(indexBuffer)));

        vertexBufferCount[meshId] = 1;
    }
    else
    {
        vertexBufferCount[meshId] = vertexBufferCount[meshId] + 1;
    }

    if (imageBuffers.count(imageId) == 0)
    {
        // TODO: only load the diffuse image for now
        std::shared_ptr<VulkanImage> diffuseImage = std::make_shared<VulkanImage>(
            context, commandPool, imageVmaAllocator);
        std::shared_ptr<Image> imageToLoad = material->diffuseImage;
        diffuseImage->Load(imageToLoad.get(), descriptorPool, pipeline->GetImageDescriptorSetLayout());
        imageBuffers.insert(std::make_pair(imageId, std::move(diffuseImage)));

        imageBufferCount[imageId] = 1;
    }
    else
    {
        imageBufferCount[imageId] = imageBufferCount[imageId] + 1;
    }

    std::shared_ptr<VulkanBuffer> instanceBuffer = std::make_shared<VulkanBuffer>(
        context, commandPool, vmaAllocator);
    instanceBuffer->Load(
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &instanceBufferInput,
        sizeof(VulkanInstanceBufferInput));
    instanceBuffer->CreateDescriptorSet(
        descriptorPool,
        pipeline->GetInstanceDescriptorSetLayout(),
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        sizeof(VulkanInstanceBufferInput));
    instanceBuffers.insert(std::make_pair(bufferId, std::move(instanceBuffer)));

    VulkanDrawingBuffer drawingBuffer{};
    drawingBuffer.instanceBufferId = bufferId;
    drawingBuffer.vertexBufferId = meshId;
    drawingBuffer.indexBufferId = meshId;
    drawingBuffer.imageBufferId = imageId;
    drawingBuffers.insert(std::make_pair(bufferId, drawingBuffer));

    VulkanDrawingCommand drawingCommand{};
    drawingCommand.instanceBuffer = instanceBuffers[drawingBuffer.instanceBufferId].get();
    drawingCommand.vertexBuffer = vertexBuffers[drawingBuffer.vertexBufferId].get();
    drawingCommand.indexBuffer = indexBuffers[drawingBuffer.indexBufferId].get();
    drawingCommand.imageBuffer = imageBuffers[drawingBuffer.imageBufferId].get();
    drawingCommandCache.insert(std::make_pair(bufferId, drawingCommand));

    return bufferId;
}

void VulkanBufferManager::Submit(uint32_t &imageIndex)
{
    VkDevice logicalDevice = context->GetLogicalDevice();
    VkQueue graphicsQueue = context->GetGraphicsQueue();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkCommandBuffer commandBuffer = commandBuffers[imageIndex]->GetBuffer();
    VkSemaphore semaphoresToWaitFor[] = { imageAvailableSemaphores[currentInFlightFrame] };
    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentInFlightFrame] };
    VkPipelineStageFlags stagesToWaitFor[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = semaphoresToWaitFor;
    submitInfo.pWaitDstStageMask = stagesToWaitFor;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    ASSERT_VK_RESULT_SUCCESS(
        vkResetFences(logicalDevice, 1, &inFlightFences[currentInFlightFrame]),
        "Failed to reset fences");

    ASSERT_VK_RESULT_SUCCESS(
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentInFlightFrame]),
        "Failed to submit the command for drawing the buffer");
}

void VulkanBufferManager::UnloadBuffer(uint32_t bufferId)
{
    if (drawingBuffers.count(bufferId) > 0)
    {
        VulkanDrawingBuffer drawingBuffer = drawingBuffers[bufferId];
        
        uint32_t instanceBufferId = drawingBuffer.instanceBufferId;
        std::shared_ptr<VulkanBuffer> instanceBuffer = instanceBuffers[instanceBufferId];
        instanceBuffer->Unload();
        instanceBuffers.erase(instanceBufferId);

        uint32_t vertexBufferId = drawingBuffer.vertexBufferId;
        vertexBufferCount[vertexBufferId] = vertexBufferCount[vertexBufferId] - 1;
        if (vertexBufferCount[vertexBufferId] == 0)
        {
            std::shared_ptr<VulkanBuffer> vertexBuffer = vertexBuffers[vertexBufferId];
            vertexBuffer->Unload();
            vertexBuffers.erase(vertexBufferId);

            std::shared_ptr<VulkanBuffer> indexBuffer = indexBuffers[vertexBufferId];
            indexBuffer->Unload();
            indexBuffers.erase(vertexBufferId);
        }

        uint32_t imageBufferId = drawingBuffer.imageBufferId;
        imageBufferCount[imageBufferId] = imageBufferCount[imageBufferId] - 1;
        if (imageBufferCount[imageBufferId] == 0)
        {
            std::shared_ptr<VulkanImage> imageBuffer = imageBuffers[imageBufferId];
            imageBuffer->Unload();
            imageBuffers.erase(imageBufferId);
        }

        drawingBuffers.erase(bufferId);
        drawingCommandCache.erase(bufferId);
    }
}

void VulkanBufferManager::UpdateUniformBuffer(VulkanUniformBufferInput input)
{
    uniformBufferInput = input;
    uniformBufferUpdated = true;
}

void VulkanBufferManager::CreateCommandBuffers()
{
    commandBuffers.resize(frameBuffers.size());
    for (uint32_t i = 0; i < commandBuffers.size(); i++)
    {
        std::unique_ptr<VulkanCommand> commandBuffer = std::make_unique<VulkanDefaultRenderCommand>(
            context,
            pipeline,
            commandPool,
            drawingCommandCache,
            uniformBuffers[i].get());
        commandBuffer->Create();
        commandBuffers[i] = std::move(commandBuffer);
    }
}

void VulkanBufferManager::CreateCommandPool()
{
    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = context->GetGraphicsQueueIndex();
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_VK_RESULT_SUCCESS(
        vkCreateCommandPool(context->GetLogicalDevice(), &commandPoolInfo, nullptr, &commandPool),
        "Failed to create command pool");
}

void VulkanBufferManager::CreateDescriptorPool()
{
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = STATIC_PIPELINE_DESCRIPTOR_POOL_SIZE_COUNT;
    poolInfo.pPoolSizes = STATIC_PIPELINE_DESCRIPTOR_POOL_SIZES;
    poolInfo.maxSets = MAX_DESCRIPTOR_SETS;
    ASSERT_VK_RESULT_SUCCESS(
        vkCreateDescriptorPool(context->GetLogicalDevice(), &poolInfo, nullptr, &descriptorPool),
        "Failed to create descriptor pool");
}

void VulkanBufferManager::CreateFrameBuffers()
{
    VkRenderPass renderPass = pipeline->GetRenderPass();
    VkExtent2D swapChainExtent = context->GetSwapChainExtent();
    VkImageView colorImageView = context->GetColorImageView();
    VkImageView depthImageView = context->GetDepthImageView();
    std::vector<VkImageView> swapChainImageViews = context->GetSwapChainImageViews();
    for (VkImageView swapChainImageView : swapChainImageViews)
    {
        VkFramebuffer frameBuffer;
        VkImageView attachments[] =
        {
            colorImageView,
            depthImageView,
            swapChainImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 3;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        ASSERT_VK_RESULT_SUCCESS(
            vkCreateFramebuffer(context->GetLogicalDevice(), &framebufferInfo, nullptr, &frameBuffer),
            "Failed to create frame buffer");

        frameBuffers.push_back(frameBuffer);
    }
}

void VulkanBufferManager::CreateMemoryAllocator()
{
    VmaAllocatorCreateInfo createInfo{};
    createInfo.device = context->GetLogicalDevice();
    createInfo.physicalDevice = context->GetPhysicalDevice();
    createInfo.vulkanApiVersion = VK_API_VERSION_1_0;
    createInfo.instance = context->GetInstance();

    ASSERT_VK_RESULT_SUCCESS(
        vmaCreateAllocator(&createInfo, &vmaAllocator),
        "Failed to create VMA allocator");
    ASSERT_VK_RESULT_SUCCESS(
        vmaCreateAllocator(&createInfo, &imageVmaAllocator),
        "Failed to create image VMA allocator");
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
        ASSERT_VK_RESULT_SUCCESS(
            vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]),
            "Failed to create synchronization objects for a frame");
        ASSERT_VK_RESULT_SUCCESS(
            vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]),
            "Failed to create synchronization objects for a frame");
        ASSERT_VK_RESULT_SUCCESS(
            vkCreateFence(logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]),
            "Failed to create synchronization objects for a frame");
    }
}

void VulkanBufferManager::CreateUniformBuffers()
{
    for (uint32_t i = 0; i < frameBuffers.size(); i++)
    {
        std::unique_ptr<VulkanBuffer> uniformBuffer = std::make_unique<VulkanBuffer>(context, commandPool, vmaAllocator);
        VulkanUniformBufferInput defaultUniformBufferInput{};
        uniformBuffer->Load(
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &defaultUniformBufferInput,
            sizeof(VulkanUniformBufferInput));
        uniformBuffer->CreateDescriptorSet(
            descriptorPool,
            pipeline->GetUniformDescriptorSetLayout(),
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            sizeof(VulkanUniformBufferInput));
        uniformBuffers.push_back(std::move(uniformBuffer));
    }
}

void VulkanBufferManager::DestroyCommandBuffers()
{
    for (std::unique_ptr<VulkanCommand> &commandBuffer : commandBuffers)
    {
        commandBuffer->Destroy();
    }
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
    for (std::unique_ptr<VulkanBuffer> &uniformBuffer : uniformBuffers)
    {
        uniformBuffer->Unload();
    }
    uniformBuffers.clear();
}

uint32_t VulkanBufferManager::GenerateBufferId()
{
    uint32_t nextBufferId;
    do
    {
        nextBufferId = distribution(generator);
    } while (instanceBuffers.count(nextBufferId) != 0);
    return nextBufferId;
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
}

#include <algorithm>
#include <execution>

#include "Common/Logger.h"
#include "Engine/Camera.h"
#include "Engine/Entity.h"
#include "Engine/Image.h"
#include "Engine/Material.h"
#include "Engine/Mesh.h"
#include "Engine/Terrain.h"
#include "VulkanDrawEngine.h"
#include "VulkanRenderPass.h"
#include "Buffer/VulkanBuffer.h"
#include "Buffer/VulkanBufferManager.h"
#include "Command/VulkanCommandPool.h"
#include "Command/VulkanCommandManager.h"
#include "Frame/VulkanFrameBuffer.h"
#include "Pipeline/VulkanPipelineManager.h"

VulkanDrawEngine::VulkanDrawEngine(Screen *screen, bool enableDebugging)
    : context(),
      screen(screen),
      isInitialized(false),
      enableDebugging(enableDebugging),
      currentInFlightFrame(0),
      pushConstants{}
{
}

VulkanDrawEngine::~VulkanDrawEngine()
{
}

void VulkanDrawEngine::Destroy()
{
    if (!isInitialized)
    {
        return;
    }

    context->WaitIdle();

    DestroyCommandBuffers();
    ClearDrawingBuffers();

    DestroySynchronizationObjects();
    DestroyFrameBuffers();

    vmaDestroyAllocator(frameBufferImageAllocator);

    commandPool->Destroy();
    pipelineManager->Destroy();
    renderPass->Destroy();
    context->Destroy();
}

void VulkanDrawEngine::DestroySynchronizationObjects()
{
    VkDevice logicalDevice = context->GetLogicalDevice();
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(logicalDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(logicalDevice, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(logicalDevice, inFlightFences[i], nullptr);
    }
}

void VulkanDrawEngine::DrawFrame()
{
    uint32_t imageIndex = 0;
    BeginFrame(imageIndex);
    Submit(imageIndex);
    EndFrame(imageIndex);
}

void VulkanDrawEngine::Initialize()
{
    context = std::make_unique<VulkanContext>(screen, enableDebugging);
    context->Create();

    renderPass = std::make_unique<VulkanRenderPass>(context.get());
    renderPass->Create();

    pipelineManager = std::make_unique<VulkanPipelineManager>(context.get(), renderPass.get());
    pipelineManager->Create();

    commandPool = std::make_unique<VulkanCommandPool>(context.get());
    commandPool->Create();

    VmaAllocatorCreateInfo vmaCreateInto{};
    vmaCreateInto.device = context->GetLogicalDevice();
    vmaCreateInto.physicalDevice = context->GetPhysicalDevice();
    vmaCreateInto.vulkanApiVersion = VK_API_VERSION_1_0;
    vmaCreateInto.instance = context->GetInstance();
    ASSERT_VK_RESULT_SUCCESS(
        vmaCreateAllocator(&vmaCreateInto, &frameBufferImageAllocator),
        "Failed to create frame buffer image VMA allocator");

    CreateFrameBuffers();
    CreateSynchronizationObjects();

    CreateCommandBuffers();
    VkCommandPool commandPoolToUse = commandPool->GetOrCreateCommandPool(std::this_thread::get_id());
    bufferManager = std::make_unique<VulkanBufferManager>(
        context.get(),
        renderPass.get(),
        pipelineManager->GetDrawingPipelines(),
        commandPoolToUse,
        static_cast<uint32_t>(screenFrameBuffers.size()));
    bufferManager->Create();

    isInitialized = true;
}

void VulkanDrawEngine::BeginFrame(uint32_t &imageIndex)
{
    // Reset command buffer of previous image index
    commandManager->Reset(imageIndex);

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
        RecreateSwapChain();
        return;
    }

    // The acquire image is still in use
    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        ASSERT_VK_RESULT_SUCCESS(
            vkWaitForFences(logicalDevice, 1, &imagesInFlight[currentInFlightFrame], VK_TRUE, UINT64_MAX),
            "Failed to wait for fences");
    }
    imagesInFlight[imageIndex] = inFlightFences[currentInFlightFrame];

    // Recording should happen only if the data are changed
    if (dataUpdated[imageIndex])
    {
        commandManager->Record(
            imageIndex,
            screenFrameBuffers[imageIndex]->GetFrameBuffer(),
            pushConstants,
            bufferManager->GetDrawingBuffer(imageIndex));
        dataUpdated[imageIndex] = false;
    }
}

// This method should only be used for cleaning up
// any remaining buffers from the GPU when the game session ends
// Do not rely on this method for unloading blocks in normal circumstances
void VulkanDrawEngine::ClearDrawingBuffers()
{
    for (uint32_t bufferId : bufferIds)
    {
        bufferManager->UnloadBuffer(bufferId);
    }
    bufferIds.clear();

    for (uint32_t terrainId : terrainBufferIds)
    {
        bufferManager->UnloadTerrainBuffer(terrainId);
    }
    terrainBufferIds.clear();

    for (uint32_t screenObjectId : screenObjectIds)
    {
        bufferManager->UnloadScreenObjectBuffer(screenObjectId);
    }
    screenObjectIds.clear();

    bufferManager->Destroy();
}

void VulkanDrawEngine::CreateCommandBuffers()
{
    commandManager = std::make_unique<VulkanCommandManager>(
        context.get(),
        commandPool.get(),
        renderPass.get(),
        pipelineManager->GetDrawingPipelines());
    commandManager->Create(static_cast<uint32_t>(screenFrameBuffers.size()));

    dataUpdated.resize(screenFrameBuffers.size());
    MarkDataAsUpdated();
}

void VulkanDrawEngine::CreateFrameBuffers()
{
    VkExtent2D swapChainExtent = context->GetSwapChainExtent();
    VkCommandPool commandPoolToUse = commandPool->GetOrCreateCommandPool(std::this_thread::get_id());

    std::vector<VkImageView> swapChainImageViews = context->GetSwapChainImageViews();
    for (uint32_t swapChainImageIndex = 0; swapChainImageIndex < swapChainImageViews.size(); swapChainImageIndex++)
    {
        std::unique_ptr<VulkanFrameBuffer> frameBuffer = std::make_unique<VulkanFrameBuffer>(
            context.get(),
            commandPoolToUse,
            frameBufferImageAllocator);
        frameBuffer->Create(
            swapChainExtent.width,
            swapChainExtent.height,
            renderPass.get(),
            swapChainImageIndex);
        screenFrameBuffers.push_back(std::move(frameBuffer));
    }
}

void VulkanDrawEngine::CreateSynchronizationObjects()
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

void VulkanDrawEngine::DestroyCommandBuffers()
{
    commandManager->Destroy();
}

void VulkanDrawEngine::DestroyFrameBuffers()
{
    for (auto const &screenFrameBuffer : screenFrameBuffers)
    {
        screenFrameBuffer->Destroy();
    }
    screenFrameBuffers.clear();
}

void VulkanDrawEngine::EndFrame(uint32_t &imageIndex)
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
        RecreateSwapChain();
    }

    currentInFlightFrame = (currentInFlightFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanDrawEngine::MarkDataAsUpdated()
{
    // This method should be called only when there are objects add/removed
    // No need to call this when only the position/color of an object is changed
    std::fill(dataUpdated.begin(), dataUpdated.end(), true);
}

void VulkanDrawEngine::LoadCubeMap(CubeMap &cubeMap)
{
    std::vector<Vertex> transformedVertices(cubeMap.vertices);
    std::transform(
        std::execution::par,
        cubeMap.vertices.begin(),
        cubeMap.vertices.end(),
        transformedVertices.begin(),
        [&](Vertex &vertex)
        {
            return ConvertToVulkanVertex(vertex);
        });
    bufferManager->LoadCubeMapBuffer(
        transformedVertices,
        cubeMap.indices,
        cubeMap.images);

    std::vector<uint8_t> skyColor = cubeMap.images[0]->GetDominantColor();
    pushConstants.meshPushConstant.fogColor =
    {
        skyColor[0] / 255.f,
        skyColor[1] / 255.f,
        skyColor[2] / 255.f
    };
}

void VulkanDrawEngine::LoadEntity(const Entity &entity)
{
    std::shared_ptr<Mesh> mesh = entity.mesh;
    glm::vec3 translation = ConvertToVulkanCoordinates(entity.translation);
    glm::vec3 scale = ConvertToVulkanCoordinates(entity.scale);
    glm::vec3 rotations = ConvertToVulkanCoordinates(entity.rotation);

    std::vector<Vertex> &vertices = mesh->vertices;
    std::vector<Vertex> transformedVertices(vertices.size());
    std::transform(
        std::execution::par,
        vertices.begin(),
        vertices.end(),
        transformedVertices.begin(),
        [&](Vertex &vertex)
        {
            return ConvertToVulkanVertex(vertex);
        });

    glm::mat4 translatedMatrix = ComputeTransformationMatrix(translation, scale, rotations);

    VulkanInstanceBufferInput instanceBufferInput{};
    instanceBufferInput.transformation = translatedMatrix;

    uint32_t entityId = entity.id;
    bufferManager->LoadIntoBuffer(
        entityId,
        mesh->id,
        mesh->material->id,
        instanceBufferInput,
        transformedVertices,
        mesh->indices,
        mesh->material.get());
    bufferIds.insert(entityId);

    MarkDataAsUpdated();
}

void VulkanDrawEngine::LoadLineSegments(std::vector<LineSegmentVertex> &lines)
{
    std::vector<LineSegmentVertex> transformedVertices(lines.size());
    std::transform(
        std::execution::par,
        lines.begin(),
        lines.end(),
        transformedVertices.begin(),
        [&](LineSegmentVertex &vertex)
        {
            return LineSegmentVertex
            {
                vertex.color,
                ConvertToVulkanCoordinates(vertex.position)
            };
        });

    bufferManager->LoadLineBuffer(transformedVertices);
}

void VulkanDrawEngine::LoadScreenObject(ScreenMesh &screenMesh)
{
    uint32_t screenObjectId = screenMesh.id;
    std::vector<ScreenObjectVertex> &vertices = screenMesh.vertices;

    bufferManager->LoadScreenObjectBuffer(
        screenObjectId,
        vertices,
        screenMesh.image.get());

    MarkDataAsUpdated();
}

void VulkanDrawEngine::LoadTerrain(Terrain &terrain)
{
    uint32_t terrainId = terrain.id;
    std::vector<Vertex> &vertices = terrain.vertices;
    std::vector<Vertex> transformedVertices(vertices.size());
    std::transform(
        std::execution::par,
        vertices.begin(),
        vertices.end(),
        transformedVertices.begin(),
        [&](Vertex &vertex)
        {
            return ConvertToVulkanVertex(vertex);
        });

    bufferManager->LoadTerrainIntoBuffer(
        terrainId,
        transformedVertices,
        terrain.indices,
        terrain.texture.get());
    terrainBufferIds.insert(terrainId);

    MarkDataAsUpdated();
}

void VulkanDrawEngine::RecreateSwapChain()
{
    Screen *screen = context->GetScreen();
    int newWidth = screen->GetWidth(),
        newHeight = screen->GetHeight();
    // Do nothing when the window is minimized
    // TODO: this loop may block the entire game,
    // need to have a fix (ex. auto-pause the game)
    while (newWidth == 0
        || newHeight == 0
        || !screen->IsActive())
    {
        newWidth = screen->GetWidth();
        newHeight = screen->GetHeight();
        screen->PollEvent();
    }
    context->WaitIdle();

    DestroyCommandBuffers();
    DestroyFrameBuffers();
    commandPool->Destroy();

    context->RecreateSwapChain();

    commandPool->Create();
    VkCommandPool commandPoolToUse = commandPool->GetOrCreateCommandPool(std::this_thread::get_id());

    bufferManager->ResetCommandPool(commandPoolToUse);
    bufferManager->ResetScreenBuffers();

    CreateFrameBuffers();
    CreateCommandBuffers();
}

void VulkanDrawEngine::SetFog(float density, float gradient)
{
    // Fog color is already set when the cubemap image is being loaded/updated
    pushConstants.meshPushConstant.fogDensity = density;
    pushConstants.meshPushConstant.fogGradient = gradient;

    MarkDataAsUpdated();
}

void VulkanDrawEngine::Submit(uint32_t &imageIndex)
{
    // Update the uniform and instance buffers before submission
    bufferManager->UpdateUniformBuffer(uniformBufferInput, imageIndex);
    for (auto [entityId, instanceBufferInput] : instanceBufferInputs)
    {
        bufferManager->UpdateInstanceBuffer(entityId, instanceBufferInput, imageIndex);
    }

    VkDevice logicalDevice = context->GetLogicalDevice();
    VkQueue graphicsQueue = context->GetGraphicsQueue();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkCommandBuffer commandBuffer = commandManager->GetCommandBuffer(imageIndex);
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

void VulkanDrawEngine::UpdateCamera(Camera *camera)
{
    glm::vec3 position = ConvertToVulkanCoordinates(camera->GetPosition());
    glm::vec3 front = ConvertToVulkanCoordinates(camera->GetFront());
    glm::vec3 up = ConvertToVulkanCoordinates(camera->GetUp());

    uniformBufferInput.projection = glm::perspective(
        camera->GetFieldOfView(),
        camera->GetAspect(),
        camera->GetZNear(),
        camera->GetZFar());
    // Conversion required for Vulkan depth range
    uniformBufferInput.projection[1][1] *= -1;
    uniformBufferInput.view = glm::lookAt(position, position + front, up);
    uniformBufferInput.lightPosition = { 100.0f, 100.0f, 100.0f };
    uniformBufferInput.eyePosition = position;
}

void VulkanDrawEngine::UpdateEntityTransformation(uint32_t entityId, EntityTransformation transformation)
{
    VulkanInstanceBufferInput input{};
    EntityTransformationMode mode = transformation.mode;

    if (mode == EntityTransformationMode::EulerAngles)
    {
        input.transformation = ComputeTransformationMatrix(
            ConvertToVulkanCoordinates(transformation.translation),
            ConvertToVulkanCoordinates(transformation.scale),
            ConvertToVulkanCoordinates(transformation.rotation));
    }
    else if (mode == EntityTransformationMode::Matrix)
    {
        input.transformation = ConvertToVulkanTransformationMatrix(transformation.matrix);
    }
    else if (mode == EntityTransformationMode::Quaternion)
    {
        input.transformation = ComputeTransformationMatrix(
            ConvertToVulkanCoordinates(transformation.translation),
            ConvertToVulkanCoordinates(transformation.scale),
            glm::normalize(ConvertToVulkanCoordinates(transformation.rotationAxis)),
            transformation.angle);
    }

    instanceBufferInputs[entityId] = input;
}

void VulkanDrawEngine::UnloadEntity(uint32_t entityId)
{
    // Ensure that the buffers are not in use by command buffers before destroying
    context->WaitIdle();

    if (bufferIds.count(entityId) > 0)
    {
        bufferManager->UnloadBuffer(entityId);
        bufferIds.erase(entityId);
    }

    instanceBufferInputs.erase(entityId);

    MarkDataAsUpdated();
}

void VulkanDrawEngine::UnloadScreenObject(uint32_t screenMeshId)
{
    context->WaitIdle();
    bufferManager->UnloadScreenObjectBuffer(screenMeshId);
    MarkDataAsUpdated();
}

void VulkanDrawEngine::UnloadTerrain(uint32_t terrainId)
{
    context->WaitIdle();

    if (terrainBufferIds.count(terrainId) > 0)
    {
        bufferManager->UnloadTerrainBuffer(terrainId);
        terrainBufferIds.erase(terrainId);
    }

    MarkDataAsUpdated();
}

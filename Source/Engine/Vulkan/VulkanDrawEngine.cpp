#include <algorithm>
#include <execution>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Common/Logger.h"
#include "Engine/Camera.h"
#include "Engine/Entity.h"
#include "VulkanCommon.h"
#include "VulkanDrawEngine.h"
#include "Buffer/VulkanBufferManager.h"
#include "Command/VulkanCommandManager.h"

VulkanDrawEngine::VulkanDrawEngine(Screen *screen, bool enableDebugging)
    : context(),
      screen(screen),
      enableDebugging(enableDebugging),
      currentInFlightFrame(0),
      bufferIds()
{
}

VulkanDrawEngine::~VulkanDrawEngine()
{
}

void VulkanDrawEngine::Destroy()
{
    context->WaitIdle();

    DestroyCommandBuffers();
    for (uint32_t bufferId : bufferIds)
    {
        bufferManager->UnloadBuffer(bufferId);
    }
    bufferIds.clear();
    bufferManager->Destroy();

    DestroySynchronizationObjects();
    DestroyFrameBuffers();
    DestroyPipelines();

    renderPass->Destroy();
    context->Destroy();
}

void VulkanDrawEngine::DestroyPipelines()
{
    cubeMapPipeline->Destroy();
    staticPipeline->Destroy();
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
    uint32_t imageIndex;
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

    CreatePipelines();
    CreateFrameBuffers();
    CreateSynchronizationObjects();

    VulkanDrawingPipelines pipelines{};
    pipelines.staticPipeline = staticPipeline.get();
    pipelines.cubeMapPipeline = cubeMapPipeline.get();

    CreateCommandBuffers();
    VkCommandPool commandPool = commandManager->GetOrCreateCommandPool(std::this_thread::get_id());
    bufferManager = std::make_unique<VulkanBufferManager>(
        context.get(),
        renderPass.get(),
        pipelines,
        commandPool,
        static_cast<uint32_t>(frameBuffers.size()));
    bufferManager->Create();
}

void VulkanDrawEngine::BeginFrame(uint32_t &imageIndex)
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
    commandManager->Record(
        imageIndex,
        frameBuffers[imageIndex],
        bufferManager->GetUniformBuffer(imageIndex),
        bufferManager->GetCubeMapBuffer(),
        bufferManager->GetDrawingCommands());
}

void VulkanDrawEngine::CreateCommandBuffers()
{
    VulkanDrawingPipelines pipelines{};
    pipelines.staticPipeline = staticPipeline.get();
    pipelines.cubeMapPipeline = cubeMapPipeline.get();

    commandManager = std::make_unique<VulkanCommandManager>(
        context.get(),
        renderPass.get(),
        pipelines);
    commandManager->Create(static_cast<uint32_t>(frameBuffers.size()));
}

void VulkanDrawEngine::CreateFrameBuffers()
{
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
        framebufferInfo.renderPass = renderPass->GetRenderPass();
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

void VulkanDrawEngine::CreatePipelines()
{
    VulkanShader staticVertexShader(context.get(), VulkanShaderType::Vertex);
    VulkanShader staticFragmentShader(context.get(), VulkanShaderType::Fragment);
    if (!staticVertexShader.Compile(STATIC_PIPELINE_VERTEX_SHADER)
        || !staticFragmentShader.Compile(STATIC_PIPELINE_FRAGMENT_SHADER))
    {
        throw std::runtime_error("Failed to compile static scene shader code");
    }
    staticVertexShader.Load();
    staticFragmentShader.Load();

    VulkanShader cubeMapVertexShader(context.get(), VulkanShaderType::Vertex);
    VulkanShader cubeMapFragmentShader(context.get(), VulkanShaderType::Fragment);
    if (!cubeMapVertexShader.Compile(CUBEMAP_PIPELINE_VERTEX_SHADER)
        || !cubeMapFragmentShader.Compile(CUBEMAP_PIPELINE_FRAGMENT_SHADER))
    {
        throw std::runtime_error("Failed to compile static scene shader code");
    }
    cubeMapVertexShader.Load();
    cubeMapFragmentShader.Load();

    VulkanPipelineConfig staticPipelineConfig{};
    staticPipelineConfig.vertexShader = &staticVertexShader;
    staticPipelineConfig.fragmentShader = &staticFragmentShader;
    staticPipelineConfig.cullMode = VK_CULL_MODE_BACK_BIT;
    staticPipelineConfig.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    staticPipeline = std::make_unique<VulkanPipeline>(context.get(), renderPass.get());
    staticPipeline->Create(staticPipelineConfig);

    VulkanPipelineConfig cubeMapPipelineConfig{};
    cubeMapPipelineConfig.vertexShader = &cubeMapVertexShader;
    cubeMapPipelineConfig.fragmentShader = &cubeMapFragmentShader;
    cubeMapPipelineConfig.cullMode = VK_CULL_MODE_NONE;
    cubeMapPipelineConfig.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    cubeMapPipeline = std::make_unique<VulkanPipeline>(context.get(), renderPass.get());
    cubeMapPipeline->Create(cubeMapPipelineConfig);

    staticVertexShader.Unload();
    staticFragmentShader.Unload();
    cubeMapVertexShader.Unload();
    cubeMapFragmentShader.Unload();
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
    VkDevice logicalDevice = context->GetLogicalDevice();
    for (VkFramebuffer frameBuffer : frameBuffers)
    {
        vkDestroyFramebuffer(logicalDevice, frameBuffer, nullptr);
    }
    frameBuffers.clear();
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

void VulkanDrawEngine::LoadCubeMap(CubeMap &cubeMap)
{
    bufferManager->UpdateCubeMapImage(cubeMap.images);
}

void VulkanDrawEngine::LoadIntoBuffer(Entity &entity)
{
    std::shared_ptr<Mesh> mesh = entity.mesh;
    glm::vec3 translation = ConvertToVulkanCoordinates(entity.translation);
    glm::vec3 scale = ConvertToVulkanCoordinates(entity.scale);

    std::vector<Vertex> vertices = mesh->vertices;
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

    glm::mat4 identitiyMatrix = glm::identity<glm::mat4>();
    glm::mat4 translatedMatrix = glm::translate(identitiyMatrix, translation);
    translatedMatrix = glm::scale(translatedMatrix, scale);

    VulkanInstanceBufferInput instanceBufferInput{};
    instanceBufferInput.transformation = translatedMatrix;

    uint32_t bufferId = bufferManager->LoadIntoBuffer(
        mesh->id,
        mesh->material->id,
        instanceBufferInput,
        transformedVertices,
        mesh->indices,
        mesh->material.get());
    bufferIds.insert(bufferId);

    for (uint32_t i = 0; i < frameBuffers.size(); i++)
    {
        commandManager->TriggerUpdate(i);
    }
}

void VulkanDrawEngine::RecreateSwapChain()
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
    }
    context->WaitIdle();

    DestroyCommandBuffers();
    DestroyFrameBuffers();

    context->RecreateSwapChain();

    CreateFrameBuffers();
    CreateCommandBuffers();
}

void VulkanDrawEngine::Submit(uint32_t &imageIndex)
{
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
    glm::vec3 target = ConvertToVulkanCoordinates(camera->GetTarget());
    glm::vec3 up = ConvertToVulkanCoordinates(camera->GetUp());

    VulkanUniformBufferInput input{};
    input.projection = glm::perspective(
        camera->GetFieldOfView(),
        camera->GetAspect(),
        camera->GetZNear(),
        camera->GetZFar());
    // Conversion required for Vulkan depth range
    input.projection[1][1] *= -1;
    input.view = glm::lookAt(position, target, up);
    input.lightPosition = { 10.0f, 10.0f, 10.0f };
    input.eyePosition = position;

    bufferManager->UpdateUniformBuffer(input);
}

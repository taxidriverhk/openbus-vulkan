#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

#include "VulkanContext.h"
#include "VulkanPipeline.h"

class VulkanBufferManager
{
public:
    VulkanBufferManager(VulkanContext *context);
    ~VulkanBufferManager();

    // Initialization
    void BindPipeline(VulkanPipeline *pipeline);
    void Create();
    void Destroy();

    // Vertex/Texture Buffering

    // Buffer Drawing
    void BeginFrame();
    void EndFrame();
    void Submit();

private:
    static uint32_t MAX_FRAMES_IN_FLIGHT;

    void BeginRecordCommandBuffers();
    void InitSyncObjects();

    VulkanContext *context;
    std::vector<VkFramebuffer> frameBuffers;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    uint32_t currentInFlightFrame;
    uint32_t currentImageIndex;
};

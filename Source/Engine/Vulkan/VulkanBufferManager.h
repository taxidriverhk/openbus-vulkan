#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

#include "VulkanContext.h"
#include "VulkanPipeline.h"

class VulkanBufferManager
{
public:
    VulkanBufferManager(VulkanContext *context, VulkanPipeline *pipeline);
    ~VulkanBufferManager();

    // Initialization
    void Create();
    void Destroy();

    // Vertex/Texture Buffering

    // Buffer Drawing
    void BeginFrame(uint32_t &imageIndex);
    void EndFrame(uint32_t &imageIndex);
    void Submit(uint32_t &imageIndex);

private:
    static uint32_t MAX_FRAMES_IN_FLIGHT;

    void CreateCommandBuffers();
    void CreateFrameBuffers();
    void CreateSynchronizationObjects();
    void DestroyCommandBuffers();
    void DestroyFrameBuffers();
    void RecordCommandBuffer(uint32_t index);
    void RecreateSwapChainAndBuffers();

    VulkanContext *context;
    VulkanPipeline *pipeline;
    std::vector<VkFramebuffer> frameBuffers;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    uint32_t currentInFlightFrame;
};

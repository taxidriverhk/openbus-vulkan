#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <vector>

#include "Engine/Mesh.h"
#include "VulkanBuffer.h"
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
    void LoadVertices(uint32_t bufferId, std::vector<Vertex> vertices);
    void UnloadBuffer(uint32_t bufferId);

    // Buffer Drawing
    void BeginFrame(uint32_t &imageIndex);
    void EndFrame(uint32_t &imageIndex);
    void Submit(uint32_t &imageIndex);

private:
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

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

    std::unordered_map<uint32_t, std::unique_ptr<VulkanBuffer>> vertexBuffers;
};

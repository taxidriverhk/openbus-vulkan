#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <vector>

#include "Engine/Mesh.h"
#include "Buffer/VulkanBuffer.h"
#include "Buffer/VulkanUniformBuffer.h"
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
    void LoadIntoBuffer(uint32_t bufferId, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
    void UnloadBuffer(uint32_t bufferId);
    void UpdateUniformBuffer(VulkanUniformBufferInput input);

    // Buffer Drawing
    void Draw(uint32_t &imageIndex);

private:
    static constexpr uint32_t MAX_DESCRIPTOR_SETS = 4;
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    void BeginFrame(uint32_t &imageIndex);
    void EndFrame(uint32_t &imageIndex);
    void Submit(uint32_t &imageIndex);

    void CreateCommandBuffers();
    void CreateCommandPool();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    void CreateFrameBuffers();
    void CreateSynchronizationObjects();
    void CreateUniformBuffers();
    void DestroyCommandBuffers();
    void DestroyFrameBuffers();
    void DestroyUniformBuffers();

    void RecordCommandBuffers();
    void RecreateSwapChainAndBuffers();

    VulkanContext *context;
    VulkanPipeline *pipeline;
    std::vector<VkFramebuffer> frameBuffers;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    uint32_t currentInFlightFrame;

    std::unordered_map<uint32_t, std::unique_ptr<VulkanBuffer>> vertexBuffers;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanBuffer>> indexBuffers;

    bool uniformBufferUpdated;
    VulkanUniformBufferInput uniformBufferInput;
    std::vector<std::unique_ptr<VulkanBuffer>> uniformBuffers;
};

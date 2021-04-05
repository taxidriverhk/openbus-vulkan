#pragma once

#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <vulkan/vulkan.h>

#include "vk_mem_alloc.hpp"

#include "Engine/Mesh.h"
#include "Buffer/VulkanBuffer.h"
#include "Command/VulkanCommand.h"
#include "Image/VulkanImage.h"
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
    uint32_t LoadIntoBuffer(
        std::vector<Vertex> &vertices,
        std::vector<uint32_t> &indices,
        Material &material);
    void UnloadBuffer(uint32_t bufferId);
    void UpdateUniformBuffer(VulkanUniformBufferInput input);

    // Buffer Drawing
    void Draw(uint32_t &imageIndex);

private:
    static constexpr uint32_t MAX_DESCRIPTOR_SETS = 4;
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    // Only used for buffer ID generation, not a real limit
    static constexpr uint32_t MAX_VERTEX_BUFFERS = 20000;

    void BeginFrame(uint32_t &imageIndex);
    void EndFrame(uint32_t &imageIndex);
    void Submit(uint32_t &imageIndex);

    void CreateCommandBuffers();
    void CreateCommandPool();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    void CreateFrameBuffers();
    void CreateMemoryAllocator();
    void CreateSynchronizationObjects();
    void CreateUniformBuffers();
    void DestroyCommandBuffers();
    void DestroyFrameBuffers();
    void DestroyUniformBuffers();

    uint32_t GenerateBufferId();

    void RecordCommandBuffers();
    void RecreateSwapChainAndBuffers();

    VmaAllocator vmaAllocator;
    VmaAllocator imageVmaAllocator;
    VulkanContext *context;
    VulkanPipeline *pipeline;

    VkCommandPool commandPool;
    VkDescriptorPool descriptorPool;

    // Based on number of swap chain images (which is usually 3)
    std::vector<VkFramebuffer> frameBuffers;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<std::unique_ptr<VulkanCommand>> commandBuffers;
    std::vector<VkFence> imagesInFlight;

    // Based on the maximum allowed frames in flight
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    // Active frame in use by the GPU
    uint32_t currentInFlightFrame;

    std::unordered_set<uint32_t> bufferIds;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanBuffer>> vertexBuffers;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanBuffer>> indexBuffers;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanImage>> bufferIdToImageBufferMap;

    bool uniformBufferUpdated;
    VulkanUniformBufferInput uniformBufferInput;
    std::vector<std::unique_ptr<VulkanBuffer>> uniformBuffers;

    std::default_random_engine generator;
    std::uniform_int_distribution<uint32_t> distribution;
};

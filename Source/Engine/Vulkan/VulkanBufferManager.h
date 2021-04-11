#pragma once

#include <random>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.h>

#include "vk_mem_alloc.hpp"

#include "Engine/Mesh.h"
#include "Buffer/VulkanBuffer.h"
#include "Command/VulkanCommand.h"
#include "Image/VulkanImage.h"
#include "VulkanCommon.h"
#include "VulkanContext.h"
#include "VulkanRenderPass.h"
#include "VulkanPipeline.h"

struct VulkanDrawingPipelines;
struct VulkanDrawingCommand;

class VulkanBufferManager
{
public:
    VulkanBufferManager(
        VulkanContext *context,
        VulkanRenderPass *renderPass,
        VulkanDrawingPipelines pipelines);
    ~VulkanBufferManager();

    // Initialization
    void Create();
    void Destroy();

    // Cubemap Buffering
    void UpdateCubeMapImage(std::vector<Image *>& images);

    // Vertex/Texture Buffering
    uint32_t LoadIntoBuffer(
        uint32_t meshId,
        uint32_t imageId,
        VulkanInstanceBufferInput &instanceBuffer,
        std::vector<Vertex> &vertices,
        std::vector<uint32_t> &indices,
        Material *material);
    void UnloadBuffer(uint32_t bufferId);
    void UpdateUniformBuffer(VulkanUniformBufferInput input);

    // Buffer Drawing
    void Draw(uint32_t &imageIndex);

private:
    // Should be good enough for images per scene
    static constexpr uint32_t MAX_DESCRIPTOR_SETS = 3 * 8192U;
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    // Only used for buffer ID generation, not a real limit
    static constexpr uint32_t MAX_VERTEX_BUFFERS = 20000;

    void BeginFrame(uint32_t &imageIndex);
    void EndFrame(uint32_t &imageIndex);
    void Submit(uint32_t &imageIndex);

    void CreateCommandBuffers();
    void CreateCommandPool();
    void CreateCubeMapBuffer();
    void CreateDescriptorPool();
    void CreateFrameBuffers();
    void CreateMemoryAllocator();
    void CreateSynchronizationObjects();
    void CreateUniformBuffers();
    void DestroyCommandBuffers();
    void DestroyCubeMapBuffer();
    void DestroyFrameBuffers();
    void DestroyUniformBuffers();

    uint32_t GenerateBufferId();

    void RecreateSwapChainAndBuffers();

    VulkanContext *context;
    VulkanRenderPass *renderPass;

    VmaAllocator vmaAllocator;
    VmaAllocator imageVmaAllocator;

    VulkanDrawingPipelines pipelines;

    VkCommandPool commandPool;
    VkDescriptorPool descriptorPool;

    // Based on number of swap chain images (which is usually 3)
    std::vector<VkFramebuffer> frameBuffers;
    std::vector<std::unique_ptr<VulkanCommand>> commandBuffers;
    std::vector<VkFence> imagesInFlight;

    // Based on the maximum allowed frames in flight
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    // Active frame in use by the GPU
    uint32_t currentInFlightFrame;

    // Cubemap buffers
    std::unique_ptr<VulkanBuffer> cubeMapVertexBuffer;
    std::unique_ptr<VulkanBuffer> cubeMapIndexBuffer;
    std::unique_ptr<VulkanImage> cubeMapImage;

    // Static scene buffers
    std::unordered_map<uint32_t, std::shared_ptr<VulkanBuffer>> instanceBuffers;

    std::unordered_map<uint32_t, std::shared_ptr<VulkanBuffer>> vertexBuffers;
    std::unordered_map<uint32_t, std::shared_ptr<VulkanBuffer>> indexBuffers;
    std::unordered_map<uint32_t, uint32_t> vertexBufferCount;

    std::unordered_map<uint32_t, std::shared_ptr<VulkanImage>> imageBuffers;
    std::unordered_map<uint32_t, uint32_t> imageBufferCount;

    std::unordered_map<uint32_t, VulkanDrawingBuffer> drawingBuffers;
    std::unordered_map<uint32_t, VulkanDrawingCommand> drawingCommandCache;

    bool uniformBufferUpdated;
    VulkanUniformBufferInput uniformBufferInput;
    std::vector<std::unique_ptr<VulkanBuffer>> uniformBuffers;

    std::default_random_engine generator;
    std::uniform_int_distribution<uint32_t> distribution;
};

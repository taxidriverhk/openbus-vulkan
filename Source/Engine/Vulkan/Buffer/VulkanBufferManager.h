#pragma once

#include <random>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.h>

#include "vk_mem_alloc.hpp"

#include "Engine/Mesh.h"
#include "Engine/Vulkan/VulkanCommon.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/VulkanRenderPass.h"
#include "Engine/Vulkan/VulkanPipeline.h"
#include "Engine/Vulkan/Command/VulkanCommand.h"
#include "Engine/Vulkan/Image/VulkanImage.h"
#include "VulkanBuffer.h"

struct VulkanDrawingPipelines;
struct VulkanDrawingCommand;

class VulkanBufferManager
{
public:
    VulkanBufferManager(
        VulkanContext *context,
        VulkanRenderPass *renderPass,
        VulkanDrawingPipelines pipelines,
        VkCommandPool commandPool,
        uint32_t frameBufferSize);
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

    VulkanCubeMapBuffer &GetCubeMapBuffer() { return cubeMapBufferCache; }
    VulkanBuffer *GetUniformBuffer(uint32_t index) const { return uniformBuffers[index].get(); }
    std::unordered_map<uint32_t, VulkanDrawingCommand> &GetDrawingCommands() { return drawingCommandCache; }

private:
    // Should be good enough for images per scene
    static constexpr uint32_t MAX_DESCRIPTOR_SETS = 3 * 8192U;
    // Only used for buffer ID generation, not a real limit
    static constexpr uint32_t MAX_VERTEX_BUFFERS = 20000;

    void CreateCubeMapBuffer();
    void CreateDescriptorPool();
    void CreateMemoryAllocator();
    void CreateUniformBuffers();
    void DestroyCubeMapBuffer();
    void DestroyUniformBuffers();

    uint32_t GenerateBufferId();

    uint32_t frameBufferSize;
    VulkanContext *context;
    VulkanRenderPass *renderPass;

    VmaAllocator vmaAllocator;
    VmaAllocator imageVmaAllocator;

    VulkanDrawingPipelines pipelines;

    VkCommandPool commandPool;
    VkDescriptorPool descriptorPool;

    // Cubemap buffers
    VulkanCubeMapBuffer cubeMapBufferCache;
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

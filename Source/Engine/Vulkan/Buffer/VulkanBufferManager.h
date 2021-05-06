#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "vk_mem_alloc.hpp"

#include "Engine/Vulkan/VulkanCommon.h"

class Image;
struct Material;
struct Vertex;

class VulkanBuffer;
class VulkanContext;
class VulkanImage;
class VulkanRenderPass;

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
    void ResetCommandPool(VkCommandPool commandPool);

    // Cubemap Buffering
    void LoadCubeMapBuffer(
        std::vector<Vertex> &vertices,
        std::vector<uint32_t> &indices,
        std::vector<Image *> &images);

    // Terrain Buffering
    void LoadTerrainIntoBuffer(
        uint32_t terrainId,
        std::vector<Vertex> &vertices,
        std::vector<uint32_t> &indices,
        Image *texture);
    void UnloadTerrain(uint32_t terrainId);

    // Vertex/Texture Buffering
    void LoadIntoBuffer(
        uint32_t instanceId,
        uint32_t meshId,
        uint32_t imageId,
        VulkanInstanceBufferInput &instanceBuffer,
        std::vector<Vertex> &vertices,
        std::vector<uint32_t> &indices,
        Material *material);
    void UnloadBuffer(uint32_t instanceId);
    void UpdateInstanceBuffer(uint32_t instanceId, VulkanInstanceBufferInput input);
    void UpdateUniformBuffer(VulkanUniformBufferInput input);

    VulkanDrawingBuffer GetDrawingBuffer(uint32_t imageIndex);

private:
    // Should be good enough for images per scene
    static constexpr uint32_t MAX_DESCRIPTOR_SETS = 3 * 8192U;
    // Only used for buffer ID generation, not a real limit
    static constexpr uint32_t MAX_VERTEX_BUFFERS = 20000;

    void CreateDescriptorPool();
    void CreateMemoryAllocator();
    void CreateUniformBuffers();
    void DestroyCubeMapBuffer();
    void DestroyUniformBuffers();

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

    std::unordered_map<uint32_t, VulkanEntityBufferIds> bufferIdCache;
    std::unordered_map<uint32_t, VulkanEntityBuffer> entityBufferCache;

    // Screen object buffers
    std::unordered_map<uint32_t, std::shared_ptr<VulkanBuffer>> screenObjectBuffers;

    // Terrain buffers
    std::unordered_map<uint32_t, std::shared_ptr<VulkanBuffer>> terrainVertexBuffers;
    std::unordered_map<uint32_t, std::shared_ptr<VulkanBuffer>> terrainIndexBuffers;
    std::unordered_map<uint32_t, VulkanTerrainBuffer> terrainBufferCache;

    bool uniformBufferUpdated;
    VulkanUniformBufferInput uniformBufferInput;
    std::vector<std::unique_ptr<VulkanBuffer>> uniformBuffers;
};

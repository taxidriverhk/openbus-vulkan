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
class VulkanTexture;
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
    void ResetScreenBuffers();
    void ResetCommandPool(VkCommandPool commandPool);

    // Cubemap Buffering
    void LoadCubeMapBuffer(
        std::vector<Vertex> &vertices,
        std::vector<uint32_t> &indices,
        std::vector<Image *> &images);

    // Line Buffering
    void LoadLineBuffer(std::vector<LineSegmentVertex> &lines);

    // Terrain Buffering
    void LoadTerrainIntoBuffer(
        uint32_t terrainId,
        std::vector<Vertex> &vertices,
        std::vector<uint32_t> &indices,
        Image *texture);
    void UnloadTerrainBuffer(uint32_t terrainId);

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

    void UpdateInstanceBuffer(uint32_t instanceId, VulkanInstanceBufferInput &input, uint32_t imageIndex);
    void UpdateUniformBuffer(VulkanUniformBufferInput &input, uint32_t imageIndex);

    // Screen Object Buffering
    void LoadScreenObjectBuffer(
        uint32_t screenObjectId,
        std::vector<ScreenObjectVertex> &vertices,
        Image *image);
    void UnloadScreenObjectBuffer(uint32_t screenObjectId);

    VulkanDrawingBuffer GetDrawingBuffer(uint32_t imageIndex);

private:
    // Should be good enough for images per scene
    static constexpr uint32_t MAX_DESCRIPTOR_SETS = 3 * 8192U;
    // Only used for buffer ID generation, not a real limit
    static constexpr uint32_t MAX_VERTEX_BUFFERS = 20000;
    // Reserve more space for frequently updated vertex buffers (ex. screen object, debug draw, etc.)
    static constexpr uint32_t MAX_VERTEX_BUFFER_CAPACITY = 5000 * sizeof(ScreenObjectVertex);

    void CreateDescriptorPool();
    void CreateMemoryAllocator();
    void CreateLineBuffer();
    void CreateScreenBuffers();
    void CreateUniformBuffers();
    void DestroyCubeMapBuffer();
    void DestroyScreenBuffers();
    void DestroyLineBuffer();
    void DestroyUniformBuffers();

    void DestroyTextureBuffer(uint32_t textureBufferId);

    uint32_t frameBufferSize;
    VulkanContext *context;
    VulkanRenderPass *renderPass;

    VmaAllocator vmaAllocator;
    VmaAllocator imageVmaAllocator;

    VulkanDrawingPipelines pipelines;

    VkCommandPool commandPool;
    VkDescriptorPool descriptorPool;

    // Buffer caches
    VulkanCubeMapBuffer cubeMapBufferCache;
    std::unordered_map<uint32_t, VulkanEntityBuffer> entityBufferCache;
    std::unordered_map<uint32_t, VulkanTerrainBuffer> terrainBufferCache;
    std::unordered_map<uint32_t, VulkanScreenObjectBuffer> screenObjectBufferCache;

    // Cubemap buffers
    bool cubeMapBufferLoaded;
    std::unique_ptr<VulkanBuffer> cubeMapVertexBuffer;
    std::unique_ptr<VulkanBuffer> cubeMapIndexBuffer;
    std::unique_ptr<VulkanTexture> cubeMapTexture;
    std::shared_ptr<VulkanImage> cubeMapImage;

    // Static scene buffers
    std::unordered_map<uint32_t, std::vector<std::shared_ptr<VulkanBuffer>>> instanceBuffers;

    std::unordered_map<uint32_t, std::shared_ptr<VulkanBuffer>> vertexBuffers;
    std::unordered_map<uint32_t, std::shared_ptr<VulkanBuffer>> indexBuffers;
    std::unordered_map<uint32_t, uint32_t> vertexBufferCount;

    std::unordered_map<uint32_t, VulkanEntityBufferIds> bufferIdCache;

    // Line segment buffer (a huge buffer for all line segments)
    std::unique_ptr<VulkanBuffer> lineVertexBuffer;

    // Screen object buffers
    std::unordered_map<uint32_t, std::shared_ptr<VulkanBuffer>> screenObjectBuffers;

    // Terrain buffers
    std::unordered_map<uint32_t, std::shared_ptr<VulkanBuffer>> terrainVertexBuffers;
    std::unordered_map<uint32_t, std::shared_ptr<VulkanBuffer>> terrainIndexBuffers;

    // Image buffers that are used by both scene and terrain pipelines
    std::unordered_map<uint32_t, std::unique_ptr<VulkanTexture>> textureBuffers;
    std::unordered_map<uint32_t, uint32_t> textureBufferCount;

    // Uniform buffers
    VulkanScreenBufferInput screenBufferInput;
    std::vector<std::unique_ptr<VulkanBuffer>> uniformBuffers;
    std::vector<std::unique_ptr<VulkanBuffer>> screenBuffers;
};

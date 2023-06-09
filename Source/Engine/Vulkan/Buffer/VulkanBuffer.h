#pragma once

#include <glm/glm.hpp>
#include "vk_mem_alloc.hpp"

#include "Engine/Vulkan/VulkanContext.h"

class VulkanBuffer
{
public:
    VulkanBuffer(
        VulkanContext *context,
        VkCommandPool &commandPool,
        VmaAllocator &allocator);
    ~VulkanBuffer();

    bool IsLoaded() const { return loaded; }
    uint32_t Size() const { return size; }
    VkBuffer GetBuffer() const { return buffer; }

    void BindDescriptorSet(VkCommandBuffer commandBuffer, uint32_t setNumber, VkPipelineLayout layout);
    void CreateDescriptorSet(
        VkDescriptorPool descriptorPool,
        VkDescriptorSetLayout descriptorSetLayout,
        VkDescriptorType type,
        uint32_t size);
    void Load(
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        void *data,
        uint32_t size,
        uint32_t reservedSize = 0);
    void Update(void *data, uint32_t size);
    void UpdateFast(void *data, uint32_t size);
    void Unload();

    static void CreateBuffer(
        VmaAllocator &allocator,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkDeviceSize capacity,
        VkBuffer &buffer,
        VmaAllocation &allocation);

private:
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    uint32_t size;
    uint32_t capacity;
    bool loaded;
    void *mappedMemory;

    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags properties;

    VmaAllocator &allocator;
    VmaAllocation allocation;

    VulkanContext *context;
    VkCommandPool &commandPool;
    VkBuffer buffer;

    VkDescriptorSet descriptorSet;
};

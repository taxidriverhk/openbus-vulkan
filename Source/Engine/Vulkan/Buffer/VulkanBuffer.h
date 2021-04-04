#pragma once

#include <glm/glm.hpp>
#include "vk_mem_alloc.hpp"

#include "Engine/Vulkan/VulkanContext.h"

struct VulkanUniformBufferInput
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

class VulkanBuffer
{
public:
    VulkanBuffer(
        VulkanContext *context,
        VkCommandPool &commandPool,
        VmaAllocator &allocator);
    ~VulkanBuffer();

    uint32_t Size() const { return size; }
    VkBuffer GetBuffer() const { return buffer; }

    void Load(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, void *data, uint32_t size);
    void Update(void *data, uint32_t size);
    void Unload();

private:
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    static void CreateBuffer(
        VmaAllocator &allocator,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkDeviceSize size,
        VkBuffer &buffer,
        VmaAllocation &allocation);

    uint32_t size;
    bool loaded;

    VmaAllocator &allocator;
    VmaAllocation allocation;

    VulkanContext *context;
    VkCommandPool &commandPool;
    VkBuffer buffer;
};
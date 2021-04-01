#pragma once

#include "VulkanContext.h"

class VulkanBuffer
{
public:
    VulkanBuffer(VulkanContext *context, VkCommandPool commandPool);

    VkBuffer GetBuffer() { return buffer; }

    virtual void Load() = 0;
    virtual uint32_t Size() const = 0;
    virtual void Unload() = 0;

protected:
    bool loaded;
    VulkanContext *context;
    VkCommandPool commandPool;
    VkBuffer buffer;
    VkDeviceMemory deviceMemory;

    void CopyBuffer(
        VkBuffer srcBuffer,
        VkBuffer dstBuffer,
        VkDeviceSize size);
    void CopyToStagingBuffer(
        VkDeviceMemory &stagingDeviceMemory,
        VkDeviceSize size,
        void *srcData);
    void CreateBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer &stagingBuffer,
        VkDeviceMemory &stagingDeviceMemory);
    void DestroyBuffer();
    void DestroyStagingBuffer(VkBuffer &stagingBuffer, VkDeviceMemory &stagingDeviceMemory);
};

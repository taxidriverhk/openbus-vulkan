#include "VulkanIndexBuffer.h"

VulkanIndexBuffer::VulkanIndexBuffer(
    VulkanContext *context,
    VkCommandPool commandPool,
    std::vector<uint32_t> indices)
    : VulkanBuffer(context, commandPool),
      indices(indices)
{
}

VulkanIndexBuffer::~VulkanIndexBuffer()
{
}

void VulkanIndexBuffer::Load()
{
    if (loaded)
    {
        return;
    }

    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingDeviceMemory;
    CreateBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingDeviceMemory);
    CopyToStagingBuffer(
        stagingDeviceMemory,
        bufferSize,
        indices.data());

    CreateBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        buffer,
        deviceMemory);
    CopyBuffer(stagingBuffer, buffer, bufferSize);

    DestroyStagingBuffer(stagingBuffer, stagingDeviceMemory);
    loaded = true;
}

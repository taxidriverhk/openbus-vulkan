#include "VulkanVertexBuffer.h"

VulkanVertexBuffer::VulkanVertexBuffer(
    VulkanContext *context,
    VkCommandPool commandPool,
    std::vector<Vertex> &vertices)
    : VulkanBuffer(context, commandPool),
      vertices(vertices),
      loaded(false)
{
}

VulkanVertexBuffer::~VulkanVertexBuffer()
{
}

void VulkanVertexBuffer::Load()
{
    if (loaded)
    {
        return;
    }

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

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
        vertices.data());

    CreateBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        buffer,
        deviceMemory);
    CopyBuffer(stagingBuffer, buffer, bufferSize);

    DestroyStagingBuffer(stagingBuffer, stagingDeviceMemory);
    loaded = true;
}

void VulkanVertexBuffer::Unload()
{
    if (!loaded)
    {
        return;
    }

    DestroyBuffer();
    loaded = false;
}

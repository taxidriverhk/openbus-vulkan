#include "VulkanUniformBuffer.h"

VulkanUniformBuffer::VulkanUniformBuffer(
    VulkanContext *context,
    VkCommandPool commandPool)
    : VulkanBuffer(context, commandPool)
{
}

VulkanUniformBuffer::~VulkanUniformBuffer()
{
}

void VulkanUniformBuffer::Load()
{
    if (loaded)
    {
        return;
    }

    VkDeviceSize bufferSize = Size();
    CreateBuffer(
        bufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        buffer,
        deviceMemory);
    loaded = true;
}

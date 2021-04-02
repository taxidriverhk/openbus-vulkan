#include "VulkanBuffer.h"

VulkanBuffer::VulkanBuffer(VulkanContext *context, VkCommandPool commandPool)
    : context(context),
      commandPool(commandPool),
      buffer(),
      deviceMemory(),
      loaded(false)
{
}

void VulkanBuffer::CopyBuffer(
    VkBuffer srcBuffer,
    VkBuffer dstBuffer,
    VkDeviceSize size)
{
    VkDevice logicalDevice = context->GetLogicalDevice();
    VkQueue graphicsQueue = context->GetGraphicsQueue();

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

    vkQueueWaitIdle(graphicsQueue);
    vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

void VulkanBuffer::CopyToStagingBuffer(
    VkDeviceMemory &stagingDeviceMemory,
    VkDeviceSize size,
    void *srcData)
{
    void *dstData;
    vkMapMemory(context->GetLogicalDevice(), stagingDeviceMemory, 0, size, 0, &dstData);
    memcpy(dstData, srcData, static_cast<uint32_t>(size));
    vkUnmapMemory(context->GetLogicalDevice(), stagingDeviceMemory);
}

void VulkanBuffer::CreateBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer &dstBuffer,
    VkDeviceMemory &dstDeviceMemory)
{
    VkDevice logicalDevice = context->GetLogicalDevice();
    VkPhysicalDevice physicalDevice = context->GetPhysicalDevice();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &dstBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(logicalDevice, dstBuffer, &memRequirements);
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    uint32_t memoryTypeIndex = 0;
    for (; memoryTypeIndex < memProperties.memoryTypeCount; memoryTypeIndex++)
    {
        if ((memRequirements.memoryTypeBits & (1 << memoryTypeIndex))
            && (memProperties.memoryTypes[memoryTypeIndex].propertyFlags & properties) == properties)
        {
            break;
        }
    }

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &dstDeviceMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(logicalDevice, dstBuffer, dstDeviceMemory, 0);
}

void VulkanBuffer::DestroyBuffer()
{
    VkDevice logicalDevice = context->GetLogicalDevice();
    vkDestroyBuffer(logicalDevice, buffer, nullptr);
    vkFreeMemory(logicalDevice, deviceMemory, nullptr);
}

void VulkanBuffer::DestroyStagingBuffer(VkBuffer &stagingBuffer, VkDeviceMemory &stagingDeviceMemory)
{
    VkDevice logicalDevice = context->GetLogicalDevice();
    vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(logicalDevice, stagingDeviceMemory, nullptr);
}

void VulkanBuffer::Unload()
{
    if (!loaded)
    {
        return;
    }

    DestroyBuffer();
    loaded = false;
}

void VulkanBuffer::UpdateBufferData(void *srcData, uint32_t size)
{
    VkDevice logicalDevice = context->GetLogicalDevice();
    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(size);

    void *dstData;
    vkMapMemory(logicalDevice, deviceMemory, 0, bufferSize, 0, &dstData);
    memcpy(dstData, srcData, size);
    vkUnmapMemory(logicalDevice, deviceMemory);
}

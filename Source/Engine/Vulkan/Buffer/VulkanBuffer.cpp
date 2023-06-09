#include "Engine/Vulkan/VulkanCommon.h"
#include "VulkanBuffer.h"

VulkanBuffer::VulkanBuffer(
    VulkanContext *context,
    VkCommandPool &commandPool,
    VmaAllocator &allocator)
    : context(context),
      commandPool(commandPool),
      allocator(allocator),
      allocation(),
      descriptorSet(VK_NULL_HANDLE),
      properties(VK_NULL_HANDLE),
      usage(VK_NULL_HANDLE),
      buffer(),
      loaded(false),
      mappedMemory(nullptr),
      capacity(0),
      size(0)
{
}

VulkanBuffer::~VulkanBuffer()
{
}

void VulkanBuffer::BindDescriptorSet(VkCommandBuffer commandBuffer, uint32_t setNumber, VkPipelineLayout layout)
{
    vkCmdBindDescriptorSets(
        commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, setNumber, 1, &descriptorSet, 0, nullptr);
}

void VulkanBuffer::CreateDescriptorSet(
    VkDescriptorPool descriptorPool,
    VkDescriptorSetLayout descriptorSetLayout,
    VkDescriptorType type,
    uint32_t size)
{
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

    ASSERT_VK_RESULT_SUCCESS(
        vkAllocateDescriptorSets(context->GetLogicalDevice(), &descriptorSetAllocInfo, &descriptorSet),
        "Failed to allocate uniform buffer descriptor set");

    VkDescriptorBufferInfo descriptorBufferInfo{};
    descriptorBufferInfo.buffer = buffer;
    descriptorBufferInfo.offset = 0;
    descriptorBufferInfo.range = size;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = type;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &descriptorBufferInfo;

    vkUpdateDescriptorSets(context->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanBuffer::Load(
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    void *data,
    uint32_t size,
    uint32_t reservedSize)
{
    if (loaded)
    {
        return;
    }

    this->capacity = std::max(size, reservedSize);
    VulkanBuffer::CreateBuffer(allocator, usage, properties, capacity, buffer, allocation);
    this->usage = usage;
    this->properties = properties;
    this->loaded = true;

    // No data to load, only buffer allocation is done
    if (!data)
    {
        return;
    }

    Update(data, size);
}

void VulkanBuffer::Update(void *data, uint32_t size)
{
    // If the new size is greater than the capacity, then the
    // buffer must be destroyed and created again
    if (this->size != 0 && size > this->capacity)
    {
        Unload();
        Load(usage, properties, data, size);
        return;
    }

    // Create another buffer for staging, and will be destroyed right after the
    // data is copied into the actual buffer
    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;
    VulkanBuffer::CreateBuffer(
        allocator,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        size,
        stagingBuffer,
        stagingAllocation);

    void *mappedData;
    vmaMapMemory(allocator, stagingAllocation, &mappedData);
    memcpy(mappedData, data, size);
    vmaUnmapMemory(allocator, stagingAllocation);

    CopyBuffer(stagingBuffer, buffer, size);

    vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);

    this->size = size;
    this->loaded = true;
}

void VulkanBuffer::UpdateFast(void *data, uint32_t size)
{
    if (this->size != 0 && size > this->capacity)
    {
        Unload();
        Load(usage, properties, data, size);
        return;
    }

    // Faster method for updating buffer
    // without having to map and unmap the buffer into CPU memory again
    // (mapping is expensive, just need to unmap it at destroy time)
    // But this method could have problem with animation
    if (mappedMemory == nullptr)
    {
        vmaMapMemory(allocator, allocation, &mappedMemory);
    }
    memcpy(mappedMemory, data, size);

    this->size = size;
    this->loaded = true;
}

void VulkanBuffer::Unload()
{
    if (!loaded)
    {
        return;
    }

    if (mappedMemory != nullptr)
    {
        vmaUnmapMemory(allocator, allocation);
    }

    context->WaitIdle();
    vmaDestroyBuffer(allocator, buffer, allocation);

    this->size = 0;
    this->loaded = false;
}

void VulkanBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
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

void VulkanBuffer::CreateBuffer(
    VmaAllocator &allocator,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkDeviceSize capacity,
    VkBuffer &buffer,
    VmaAllocation &allocation)
{
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.size = capacity;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocationInfo{};
    allocationInfo.requiredFlags = properties;
    ASSERT_VK_RESULT_SUCCESS(
        vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationInfo, &buffer, &allocation, nullptr),
        "Failed to allocate buffer");
}

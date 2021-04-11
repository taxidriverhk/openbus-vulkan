#include "Engine/Image.h"
#include "Engine/Vulkan/VulkanCommon.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/Buffer/VulkanBuffer.h"
#include "VulkanImage.h"

VulkanImage::VulkanImage(
    VulkanContext *context,
    VkCommandPool &commandPool,
    VmaAllocator &allocator,
    VulkanImageType type)
    : context(context),
      commandPool(commandPool),
      allocator(allocator),
      type(type),
      allocation(),
      descriptorSet(),
      format(VK_FORMAT_R8G8B8A8_SRGB),
      image(),
      imageView(),
      sampler(),
      loaded(false)
{
}

VulkanImage::~VulkanImage()
{
}

void VulkanImage::BindDescriptorSet(VkCommandBuffer commandBuffer, uint32_t setNumber, VkPipelineLayout layout)
{
    vkCmdBindDescriptorSets(
        commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, setNumber, 1, &descriptorSet, 0, nullptr);
}

void VulkanImage::Load(
    std::vector<uint8_t *> imagePixels,
    uint32_t width,
    uint32_t height,
    VkDescriptorPool descriptorPool,
    VkDescriptorSetLayout descriptorSetLayout)
{
    CreateImage(imagePixels, width, height);
    CreateImageView();
    CreateSampler();
    CreateDescriptorSet(descriptorPool, descriptorSetLayout);

    this->loaded = true;
}

void VulkanImage::Unload()
{
    if (!loaded)
    {
        return;
    }

    VkDevice logicalDevice = context->GetLogicalDevice();

    vkDestroySampler(logicalDevice, sampler, nullptr);
    vkDestroyImageView(logicalDevice, imageView, nullptr);
    vmaDestroyImage(allocator, image, allocation);

    this->loaded = false;
}

void VulkanImage::CreateImage(
    std::vector<uint8_t *> imagePixels,
    uint32_t width,
    uint32_t height)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (type == VulkanImageType::Texture)
    {
        imageInfo.arrayLayers = 1;
    }
    else if (type == VulkanImageType::CubeMap)
    {
        imageInfo.arrayLayers = 6;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    VmaAllocationCreateInfo allocationInfo{};
    allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    ASSERT_VK_RESULT_SUCCESS(
        vmaCreateImage(allocator, &imageInfo, &allocationInfo, &image, &allocation, nullptr),
        "Failed to allocate image");

    if (imagePixels.size() > 0)
    {
        UpdateImagePixels(imagePixels, width, height);
    }
}

void VulkanImage::CreateImageView()
{
    VkDevice logicalDevice = context->GetLogicalDevice();

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    if (type == VulkanImageType::Texture)
    {
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.subresourceRange.layerCount = 1;
    }
    else if (type == VulkanImageType::CubeMap)
    {
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        viewInfo.subresourceRange.layerCount = 6;
    }

    ASSERT_VK_RESULT_SUCCESS(
        vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imageView),
        "Failed to create texture image view");
}

void VulkanImage::CreateSampler()
{
    VkDevice logicalDevice = context->GetLogicalDevice();

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    ASSERT_VK_RESULT_SUCCESS(
        vkCreateSampler(logicalDevice, &samplerInfo, nullptr, &sampler),
        "Failed to create texture sampler");
}

void VulkanImage::CreateDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout)
{
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

    ASSERT_VK_RESULT_SUCCESS(
        vkAllocateDescriptorSets(context->GetLogicalDevice(), &descriptorSetAllocInfo, &descriptorSet),
        "Failed to allocate image descriptor set");

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(context->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
}

VkCommandBuffer VulkanImage::BeginSingleUseCommandBuffer()
{
    VkDevice logicalDevice = context->GetLogicalDevice();

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

    return commandBuffer;
}

void VulkanImage::CopyDataToImageBuffer(
    VkBuffer stagingBuffer,
    VkDeviceSize offset,
    uint32_t layerIndex,
    uint32_t width,
    uint32_t height)
{
    VkCommandBuffer commandBuffer = BeginSingleUseCommandBuffer();

    VkBufferImageCopy region{};
    region.bufferOffset = offset;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = layerIndex;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };
    region.imageSubresource.layerCount = 1;

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    EndSingleUseCommandBuffer(commandBuffer);
}

void VulkanImage::EndSingleUseCommandBuffer(VkCommandBuffer commandBuffer)
{
    VkDevice logicalDevice = context->GetLogicalDevice();
    VkQueue graphicsQueue = context->GetGraphicsQueue();

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

void VulkanImage::RunPipelineBarrierCommand(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = BeginSingleUseCommandBuffer();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    if (type == VulkanImageType::Texture)
    {
        barrier.subresourceRange.layerCount = 1;
    }
    else if (type == VulkanImageType::CubeMap)
    {
        barrier.subresourceRange.layerCount = 6;
    }

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
        && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw std::invalid_argument("Invalid layout transition parameters");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage,
        destinationStage,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier);

    EndSingleUseCommandBuffer(commandBuffer);
}

void VulkanImage::UpdateImagePixels(
    std::vector<uint8_t *> imagePixels,
    uint32_t width,
    uint32_t height)
{
    // TODO: VkImage must be recreated if the size has changed, otherwise it could crash the game

    RunPipelineBarrierCommand(format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkDeviceSize imageSize = 0, layerSize = 1;
    if (type == VulkanImageType::Texture)
    {
        imageSize = static_cast<VkDeviceSize>(width) * height * static_cast<VkDeviceSize>(sizeof(uint32_t));
        layerSize = imageSize;
    }
    else if (type == VulkanImageType::CubeMap)
    {
        // Cube map has six images to load into
        imageSize = 6 * static_cast<VkDeviceSize>(width) * height * static_cast<VkDeviceSize>(sizeof(uint32_t));
        layerSize = imageSize / 6;
    }

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;
    VulkanBuffer::CreateBuffer(
        allocator,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        imageSize,
        stagingBuffer,
        stagingAllocation);

    void *mappedData;
    vmaMapMemory(allocator, stagingAllocation, &mappedData);
    for (uint32_t layer = 0; layer < imagePixels.size(); layer++)
    {
        uint8_t *pixels = imagePixels[layer];
        VkDeviceSize offset = layer * layerSize;
        memcpy(static_cast<uint8_t *>(mappedData) + offset, pixels, layerSize);
        CopyDataToImageBuffer(stagingBuffer, offset, layer, width, height);
    }
    vmaUnmapMemory(allocator, stagingAllocation);

    RunPipelineBarrierCommand(format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
}

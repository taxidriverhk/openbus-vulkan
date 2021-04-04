#pragma once

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.hpp"

class Image;
class VulkanContext;

class VulkanImage
{
public:
    VulkanImage(
        VulkanContext *context,
        VkCommandPool &commandPool,
        VmaAllocator &allocator);
    ~VulkanImage();

    VkImage GetImage() { return image; }
    VkImageView GetImageView() { return imageView; }
    VkSampler GetSampler() { return sampler; }

    void Load(Image &srcImage);
    void Unload();

private:
    void CreateImage(Image &srcImage);
    void CreateImageView();
    void CreateSampler();

    VkCommandBuffer BeginSingleUseCommandBuffer();
    void CopyDataToImageBuffer(VkBuffer stagingBuffer, uint32_t width, uint32_t height);
    void EndSingleUseCommandBuffer(VkCommandBuffer commandBuffer);
    void RunPipelineBarrierCommand(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    bool loaded;

    VulkanContext *context;
    VkCommandPool &commandPool;
    VmaAllocator &allocator;

    VkFormat format;
    VkImage image;
    VkImageView imageView;
    VkSampler sampler;
    VmaAllocation allocation;
};
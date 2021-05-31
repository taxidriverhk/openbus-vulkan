#pragma once

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.hpp"

class VulkanContext;

enum class VulkanImageType
{
    Texture,
    CubeMap,
    Color,
    Depth
};

class VulkanImage
{
public:
    VulkanImage(
        VulkanContext *context,
        VkCommandPool &commandPool,
        VmaAllocator &allocator,
        VulkanImageType type);
    ~VulkanImage();

    VkImage GetImage() { return image; }
    VkImageView GetImageView() { return imageView; }
    VkSampler GetSampler() { return sampler; }

    void Load(
        std::vector<uint8_t *> imagePixels,
        uint32_t width,
        uint32_t height);
    void Unload();
    void UpdateImagePixels(
        std::vector<uint8_t *> imagePixels,
        uint32_t imageWidth,
        uint32_t imageHeight);

private:
    void CreateImage(
        std::vector<uint8_t *> imagePixels,
        uint32_t imageWidth,
        uint32_t imageHeight);
    void CreateImageView();
    void CreateSampler();

    VkCommandBuffer BeginSingleUseCommandBuffer();
    void CopyDataToImageBuffer(
        VkBuffer stagingBuffer,
        VkDeviceSize offset,
        uint32_t layerIndex,
        uint32_t width,
        uint32_t height);
    void EndSingleUseCommandBuffer(VkCommandBuffer commandBuffer);
    
    VkFormat FindImageFormat(VulkanImageType type);
    VkImageAspectFlags GetAspectFlags(VulkanImageType type);
    VkImageUsageFlags GetUsageFlags(VulkanImageType type);

    void GenerateMipmaps(VkFormat format);
    void RunPipelineBarrierCommand(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    VulkanImageType type;
    bool loaded;

    VulkanContext *context;
    VkCommandPool &commandPool;
    VmaAllocator &allocator;

    uint32_t mipLevels;
    uint32_t width;
    uint32_t height;

    VkImage image;
    VkImageView imageView;
    VkSampler sampler;
    VmaAllocation allocation;
};

#pragma once

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.hpp"

class VulkanContext;

enum class VulkanImageType
{
    Texture,
    CubeMap
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

    void BindDescriptorSet(VkCommandBuffer commandBuffer, uint32_t setNumber, VkPipelineLayout layout);
    void Load(
        std::vector<uint8_t *> imagePixels,
        uint32_t width,
        uint32_t height,
        VkDescriptorPool descriptorPool,
        VkDescriptorSetLayout descriptorSetLayout);
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
    void CreateDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);

    VkCommandBuffer BeginSingleUseCommandBuffer();
    void CopyDataToImageBuffer(
        VkBuffer stagingBuffer,
        VkDeviceSize offset,
        uint32_t layerIndex,
        uint32_t width,
        uint32_t height);
    void EndSingleUseCommandBuffer(VkCommandBuffer commandBuffer);
    void GenerateMipmaps();
    void RunPipelineBarrierCommand(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    VulkanImageType type;
    bool loaded;

    VulkanContext *context;
    VkCommandPool &commandPool;
    VmaAllocator &allocator;

    uint32_t mipLevels;
    uint32_t width;
    uint32_t height;

    VkFormat format;
    VkImage image;
    VkImageView imageView;
    VkSampler sampler;
    VmaAllocation allocation;

    VkDescriptorSet descriptorSet;
};

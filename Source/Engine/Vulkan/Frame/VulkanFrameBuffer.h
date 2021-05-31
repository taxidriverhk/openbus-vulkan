#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.hpp"

class VulkanContext;
class VulkanImage;
class VulkanRenderPass;

class VulkanFrameBuffer
{
public:
    VulkanFrameBuffer(
        VulkanContext *context,
        VkCommandPool &commandPool,
        VmaAllocator &allocator);
    ~VulkanFrameBuffer();

    VkFramebuffer GetFrameBuffer() const { return frameBuffer; }
    VulkanImage *GetColorImage() const { return imageAttachments[0].get(); }
    VulkanImage *GetDepthImage() const { return imageAttachments[1].get(); }

    void Create(
        uint32_t width,
        uint32_t height,
        VulkanRenderPass *renderPass,
        int swapChainImageIndex);
    void Destroy();

private:
    VulkanContext *context;
    VkCommandPool &commandPool;
    VmaAllocator &allocator;

    VkFramebuffer frameBuffer;

    std::vector<std::unique_ptr<VulkanImage>> imageAttachments;
};

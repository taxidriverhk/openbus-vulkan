#include <assert.h>

#include "Engine/Vulkan/VulkanCommon.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/VulkanRenderPass.h"
#include "Engine/Vulkan/Image/VulkanImage.h"
#include "VulkanFrameBuffer.h"

VulkanFrameBuffer::VulkanFrameBuffer(
    VulkanContext *context,
    VkCommandPool &commandPool,
    VmaAllocator &allocator)
    : context(context),
      commandPool(commandPool),
      allocator(allocator),
      frameBuffer(VK_NULL_HANDLE)
{
}

VulkanFrameBuffer::~VulkanFrameBuffer()
{
}

void VulkanFrameBuffer::Create(
    uint32_t width,
    uint32_t height,
    VulkanRenderPass *renderPass,
    int swapChainImageIndex)
{
    // Create one color image and depth image by default, no pixels will be sent obviously
    // TODO: may add ability to optionally define descript pool and layout
    // so that those images can be used in a shader later (ex. shadow, reflection, etc.)
    std::vector<uint8_t *> blankImagePixels;
    std::unique_ptr<VulkanImage> colorImage = std::make_unique<VulkanImage>(
        context,
        commandPool,
        allocator,
        VulkanImageType::Color);
    colorImage->Load(blankImagePixels, width, height, VK_NULL_HANDLE, VK_NULL_HANDLE);
    std::unique_ptr<VulkanImage> depthImage = std::make_unique<VulkanImage>(
        context,
        commandPool,
        allocator,
        VulkanImageType::Depth);
    depthImage->Load(blankImagePixels, width, height, VK_NULL_HANDLE, VK_NULL_HANDLE);

    imageAttachments.push_back(std::move(colorImage));
    imageAttachments.push_back(std::move(depthImage));

    std::vector<VkImageView> frameBufferImageViews;
    for (auto const &imageAttachment : imageAttachments)
    {
        frameBufferImageViews.push_back(imageAttachment->GetImageView());
    }
    if (swapChainImageIndex >= 0)
    {
        const std::vector<VkImageView> &swapChainImageViews = context->GetSwapChainImageViews();
        assert(swapChainImageIndex < swapChainImageViews.size());
        VkImageView swapChainImageView = swapChainImageViews[swapChainImageIndex];
        frameBufferImageViews.push_back(swapChainImageView);
    }

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass->GetRenderPass();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(frameBufferImageViews.size());
    framebufferInfo.pAttachments = frameBufferImageViews.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1;

    ASSERT_VK_RESULT_SUCCESS(
        vkCreateFramebuffer(context->GetLogicalDevice(), &framebufferInfo, nullptr, &frameBuffer),
        "Failed to create frame buffer");
}

void VulkanFrameBuffer::Destroy()
{
    vkDestroyFramebuffer(context->GetLogicalDevice(), frameBuffer, nullptr);
    for (auto const &imageAttachment : imageAttachments)
    {
        imageAttachment->Unload();
    }
    imageAttachments.clear();
}

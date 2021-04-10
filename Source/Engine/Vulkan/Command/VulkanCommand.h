#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class VulkanBuffer;
class VulkanContext;
class VulkanImage;
class VulkanPipeline;
class VulkanRenderPass;

class VulkanCommand
{
public:
    VulkanCommand(
        VulkanContext *context,
        VulkanRenderPass *renderPass,
        VkCommandPool pool);
    ~VulkanCommand();

    VkCommandBuffer GetBuffer() const { return buffer; }

    void Create();
    void Destroy();
    virtual void Record(VkFramebuffer frameBuffer) = 0;

protected:
    VkCommandBuffer BeginCommandBuffer(VkFramebuffer frameBuffer);
    void BindPipeline(VulkanPipeline *pipeline);
    void EndCommandBuffer();

private:
    VulkanContext *context;
    VulkanRenderPass *renderPass;
    VkCommandBuffer buffer;
    VkCommandPool pool;
};

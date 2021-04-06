#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class VulkanBuffer;
class VulkanContext;
class VulkanImage;
class VulkanPipeline;

class VulkanCommand
{
public:
    VulkanCommand(
        VulkanContext *context,
        VulkanPipeline *pipeline,
        VkCommandPool pool);
    ~VulkanCommand();

    VkCommandBuffer GetBuffer() const { return buffer; }

    void Create();
    void Destroy();
    virtual void Record(VkFramebuffer frameBuffer) = 0;

protected:
    VulkanPipeline *GetPipeline() const { return pipeline; }

    VkCommandBuffer BeginCommandBuffer(VkFramebuffer frameBuffer);
    void BindPipeline();
    void EndCommandBuffer();

private:
    VulkanContext *context;
    VulkanPipeline *pipeline;
    VkCommandBuffer buffer;
    VkCommandPool pool;
};

#pragma once

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
        VkCommandPool pool,
        VkDescriptorSet descriptorSet);
    ~VulkanCommand();

    void Create();
    void Destroy();
    virtual void Record(VkFramebuffer frameBuffer) = 0;

protected:
    VkCommandBuffer BeginCommandBuffer(VkFramebuffer frameBuffer);
    void EndCommandBuffer();

    void BindDescriptorSets();
    void UpdateDescriptor(uint32_t binding, VulkanBuffer &dataBuffer, uint32_t size);
    void UpdateDescriptor(uint32_t binding, VulkanImage &dataImage, uint32_t size);

private:
    VulkanContext *context;
    VulkanPipeline *pipeline;
    VkDescriptorSet descriptorSet;
    VkCommandBuffer buffer;
    VkCommandPool pool;
};

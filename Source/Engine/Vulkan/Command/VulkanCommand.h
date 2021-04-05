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
        VkCommandPool pool,
        VkDescriptorSet descriptorSet);
    ~VulkanCommand();

    VkCommandBuffer GetBuffer() const { return buffer; }
    VkDescriptorSet GetDescriptorSet() const { return descriptorSet; }

    void Create();
    void Destroy();
    virtual void Record(VkFramebuffer frameBuffer) = 0;

protected:
    VkCommandBuffer BeginCommandBuffer(VkFramebuffer frameBuffer);
    void EndCommandBuffer();

    void BindDescriptorSets();
    void BindPipeline();
    void UpdateDescriptor(uint32_t binding, VulkanBuffer *dataBuffer, uint32_t size);
    void UpdateDescriptor(uint32_t binding, VulkanImage *dataImage);

private:
    VulkanContext *context;
    VulkanPipeline *pipeline;
    VkDescriptorSet descriptorSet;
    VkCommandBuffer buffer;
    VkCommandPool pool;
};

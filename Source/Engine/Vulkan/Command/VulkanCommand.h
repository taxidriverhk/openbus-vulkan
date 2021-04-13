#pragma once

#include <vulkan/vulkan.h>

class VulkanContext;

class VulkanCommand
{
public:
    VulkanCommand(VulkanContext *context, VkCommandPool pool);
    ~VulkanCommand();

    VkCommandBuffer GetBuffer() const { return buffer; }

    void Create();
    void Destroy();

private:
    VulkanContext *context;
    VkCommandBuffer buffer;
    VkCommandPool pool;
};

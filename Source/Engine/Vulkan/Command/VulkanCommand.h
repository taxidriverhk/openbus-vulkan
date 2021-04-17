#pragma once

#include <vulkan/vulkan.h>

class VulkanContext;

class VulkanCommand
{
public:
    VulkanCommand(VulkanContext *context, VkCommandPool pool);
    ~VulkanCommand();

    VkCommandBuffer GetBuffer() const { return buffer; }

    void Create(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    void Destroy();
    void Reset();

private:
    VulkanContext *context;
    VkCommandBuffer buffer;
    VkCommandPool pool;
};

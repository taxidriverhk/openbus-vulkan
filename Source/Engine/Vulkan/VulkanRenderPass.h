#pragma once

#include <vulkan/vulkan.h>

#include "VulkanContext.h"

class VulkanRenderPass
{
public:
    VulkanRenderPass(VulkanContext *context);
    ~VulkanRenderPass();

    VkRenderPass GetRenderPass() const { return renderPass; }

    void Create();
    void Destroy();

private:
    VulkanContext *context;
    VkRenderPass renderPass;
};

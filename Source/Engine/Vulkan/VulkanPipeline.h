#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

class VulkanContext;

class VulkanPipeline
{
public:
    VulkanPipeline(VulkanContext *context);
    ~VulkanPipeline();

    void Create();
    void Destroy();
private:
    VulkanContext *context;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};

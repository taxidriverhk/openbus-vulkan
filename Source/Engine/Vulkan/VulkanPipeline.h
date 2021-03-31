#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

#include "VulkanContext.h"
#include "VulkanShader.h"

class VulkanPipeline
{
public:
    VulkanPipeline(VulkanContext *context, VulkanShader *vertexShader, VulkanShader *fragmentShader);
    ~VulkanPipeline();

    VkPipeline GetPipeline() const { return pipeline; }

    void Create();
    void Destroy();
    void Recreate();

private:
    static char * const SHADER_MAIN_FUNCTION_NAME;

    VulkanShader *vertexShader;
    VulkanShader *fragmentShader;

    VulkanContext *context;

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};

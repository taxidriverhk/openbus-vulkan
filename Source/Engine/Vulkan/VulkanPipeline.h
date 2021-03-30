#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

#include "VulkanContext.h"
#include "VulkanShader.h"

class VulkanPipeline
{
public:
    VulkanPipeline(VulkanContext *context);
    ~VulkanPipeline();

    VkPipeline GetPipeline() const { return pipeline; }

    void Create(VulkanShader &vertexShader, VulkanShader &fragmentShader);
    void Destroy();

private:
    static char *const SHADER_MAIN_FUNCTION_NAME;

    VulkanContext *context;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};

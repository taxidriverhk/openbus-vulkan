#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

#include "VulkanContext.h"
#include "VulkanShader.h"

#define SHADER_MAIN_FUNCTION_NAME "main"

class VulkanPipeline
{
public:
    VulkanPipeline(VulkanContext *context);
    ~VulkanPipeline();

    void Create(VulkanShader &vertexShader, VulkanShader &fragmentShader);
    void Destroy();
private:
    VulkanContext *context;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};

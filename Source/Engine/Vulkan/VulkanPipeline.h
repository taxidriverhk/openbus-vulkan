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
    VkPipelineLayout GetPipelineLayout() const { return pipelineLayout; }
    VkDescriptorSetLayout GetDescriptorSetLayout() const { return descriptorSetLayout; }

    void Create();
    void Destroy();
    void Recreate();

private:
    static constexpr char * SHADER_MAIN_FUNCTION_NAME = "main";

    VulkanShader *vertexShader;
    VulkanShader *fragmentShader;

    VulkanContext *context;

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;

    VkDescriptorSetLayout descriptorSetLayout;
};

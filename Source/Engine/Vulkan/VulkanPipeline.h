#pragma once

#include <string>

#include <vulkan/vulkan.h>

#include "VulkanContext.h"
#include "VulkanShader.h"

class VulkanPipeline
{
public:
    VulkanPipeline(VulkanContext *context, VulkanShader *vertexShader, VulkanShader *fragmentShader);
    ~VulkanPipeline();

    VkPipeline GetPipeline() const { return pipeline; }
    VkPipelineLayout GetPipelineLayout() const { return pipelineLayout; }
    VkDescriptorSetLayout GetUniformDescriptorSetLayout() const { return uniformDescriptorSetLayout; }
    VkDescriptorSetLayout GetPerObjectDescriptorSetLayout() const { return perObjectDescriptorSetLayout; }

    void Create();
    void Destroy();
    void Recreate();

private:
    VulkanShader *vertexShader;
    VulkanShader *fragmentShader;

    VulkanContext *context;

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;

    VkDescriptorSetLayout uniformDescriptorSetLayout;
    VkDescriptorSetLayout perObjectDescriptorSetLayout;
};

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
    VkRenderPass GetRenderPass() const { return renderPass; }

    void Create();
    void Destroy();
    void Recreate();

private:
    void CreateRenderPass();

    VulkanContext *context;

    VulkanShader *vertexShader;
    VulkanShader *fragmentShader;

    VkRenderPass renderPass;

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;

    VkDescriptorSetLayout uniformDescriptorSetLayout;
    VkDescriptorSetLayout perObjectDescriptorSetLayout;
};

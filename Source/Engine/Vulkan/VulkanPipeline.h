#pragma once

#include <string>

#include <vulkan/vulkan.h>

#include "VulkanContext.h"
#include "VulkanRenderPass.h"
#include "VulkanShader.h"

class VulkanPipeline
{
public:
    VulkanPipeline(VulkanContext *context, VulkanRenderPass *renderPass);
    ~VulkanPipeline();

    VkPipeline GetPipeline() const { return pipeline; }
    VkPipelineLayout GetPipelineLayout() const { return pipelineLayout; }
    VkDescriptorSetLayout GetUniformDescriptorSetLayout() const { return uniformDescriptorSetLayout; }
    VkDescriptorSetLayout GetImageDescriptorSetLayout() const { return imageDescriptorSetLayout; }
    VkDescriptorSetLayout GetInstanceDescriptorSetLayout() const { return instanceDescriptorSetLayout; }

    void Create(VulkanShader &vertexShader, VulkanShader &fragmentShader);
    void Destroy();

private:
    VulkanContext *context;
    VulkanRenderPass *renderPass;

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;

    VkDescriptorSetLayout uniformDescriptorSetLayout;
    VkDescriptorSetLayout imageDescriptorSetLayout;
    VkDescriptorSetLayout instanceDescriptorSetLayout;
};

#pragma once

#include <string>

#include <vulkan/vulkan.h>

#include "VulkanContext.h"
#include "VulkanRenderPass.h"
#include "VulkanShader.h"

struct VulkanPipelineConfig
{
    VulkanShader *vertexShader;
    VulkanShader *fragmentShader;
    VkCullModeFlags cullMode;
    VkFrontFace frontFace;
};

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

    void Create(VulkanPipelineConfig config);
    void Destroy();

private:
    void CreateDescriptorLayouts();
    void DestroyDescriptorLayouts();

    VulkanContext *context;
    VulkanRenderPass *renderPass;

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;

    VkDescriptorSetLayout uniformDescriptorSetLayout;
    VkDescriptorSetLayout imageDescriptorSetLayout;
    VkDescriptorSetLayout instanceDescriptorSetLayout;
};

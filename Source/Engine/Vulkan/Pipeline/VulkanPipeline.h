#pragma once

#include <string>

#include <vulkan/vulkan.h>

#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/VulkanRenderPass.h"
#include "Engine/Vulkan/VulkanShader.h"

enum class VulkanDescriptorLayoutType
{
    Image,
    Instance,
    Uniform
};

struct VulkanDescriptorLayoutConfig
{
    VulkanDescriptorLayoutType type;
    uint32_t bindingCount;
    const VkDescriptorSetLayoutBinding *bindings;
};

struct VulkanPushConstantLayoutConfig
{
    VkShaderStageFlags stage;
    uint32_t size;
};

struct VulkanVertexLayoutConfig
{
    uint32_t vertexSize;
    uint32_t descriptionCount;
    const VkVertexInputAttributeDescription *descriptions;
};

struct VulkanPipelineConfig
{
    VulkanShader *vertexShader;
    VulkanShader *fragmentShader;

    VulkanVertexLayoutConfig vertexLayoutConfig;
    std::vector<VulkanDescriptorLayoutConfig> descriptorLayoutConfigs;
    std::vector<VulkanPushConstantLayoutConfig> pushConstantConfigs;

    bool depthTestEnable;
    VkCullModeFlags cullMode;
    VkFrontFace frontFace;

    VkPrimitiveTopology topology;
    VkPolygonMode polygonMode;

    bool enableColorBlending;
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
    void CreateDescriptorLayouts(
        const std::vector<VulkanDescriptorLayoutConfig> &descriptorLayoutConfigs,
        const std::vector<VulkanPushConstantLayoutConfig> &pushConstantConfigs);
    void DestroyDescriptorLayouts();

    VulkanContext *context;
    VulkanRenderPass *renderPass;

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;

    VkDescriptorSetLayout uniformDescriptorSetLayout;
    VkDescriptorSetLayout imageDescriptorSetLayout;
    VkDescriptorSetLayout instanceDescriptorSetLayout;
};

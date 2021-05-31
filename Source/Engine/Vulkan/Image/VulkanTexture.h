#pragma once

#include <memory>
#include <unordered_map>

#include <vulkan/vulkan.h>

class VulkanContext;
class VulkanImage;

class VulkanTexture
{
public:
    VulkanTexture(
        VulkanContext *context,
        VkDescriptorPool descriptorPool,
        VkDescriptorSetLayout descriptorSetLayout);
    ~VulkanTexture();

    void Create();
    void Destroy();

    std::vector<std::shared_ptr<VulkanImage>> GetImages() const;

    void AddImage(std::shared_ptr<VulkanImage> image, uint32_t descriptorSetBinding);
    void BindDescriptorSet(VkCommandBuffer commandBuffer, uint32_t setNumber, VkPipelineLayout layout);

private:
    VulkanContext *context;

    // Descriptor set binding index to image
    std::unordered_map<uint32_t, std::shared_ptr<VulkanImage>> images;

    // Descript set related values
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;
};

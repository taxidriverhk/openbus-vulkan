#include <assert.h>

#include "Engine/Vulkan/VulkanCommon.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "VulkanImage.h"
#include "VulkanTexture.h"

VulkanTexture::VulkanTexture(
    VulkanContext *context,
    VkDescriptorPool descriptorPool,
    VkDescriptorSetLayout descriptorSetLayout)
    : context(context),
      descriptorPool(descriptorPool),
      descriptorSetLayout(descriptorSetLayout),
      descriptorSet(VK_NULL_HANDLE)
{
}

VulkanTexture::~VulkanTexture()
{
}

void VulkanTexture::Create()
{
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

    ASSERT_VK_RESULT_SUCCESS(
        vkAllocateDescriptorSets(context->GetLogicalDevice(), &descriptorSetAllocInfo, &descriptorSet),
        "Failed to allocate image descriptor set");
}

void VulkanTexture::Destroy()
{
    // No need to explicitly free the descriptor sets here
    // as the buffer manager will destroy the descriptor pool which will free the sets already
    images.clear();
}

void VulkanTexture::AddImage(std::shared_ptr<VulkanImage> image, uint32_t descriptorSetBinding)
{
    assert((descriptorSet != VK_NULL_HANDLE, "Descriptor set must be created before adding an image"));

    // Maps a Vulkan image buffer to a specific binding of a descriptor set
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageView = image->GetImageView();
    imageInfo.sampler = image->GetSampler();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = descriptorSetBinding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(context->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);

    images[descriptorSetBinding] = image;
}

void VulkanTexture::BindDescriptorSet(VkCommandBuffer commandBuffer, uint32_t setNumber, VkPipelineLayout layout)
{
    vkCmdBindDescriptorSets(
        commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, setNumber, 1, &descriptorSet, 0, nullptr);
}

std::vector<std::shared_ptr<VulkanImage>> VulkanTexture::GetImages() const
{
    std::vector<std::shared_ptr<VulkanImage>> result;
    for (auto &[descriptorSetBinding, image] : images)
    {
        result.push_back(image);
    }
    return result;
}

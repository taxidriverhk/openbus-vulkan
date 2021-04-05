#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/VulkanPipeline.h"
#include "Engine/Vulkan/Buffer/VulkanBuffer.h"
#include "Engine/Vulkan/Image/VulkanImage.h"
#include "VulkanCommand.h"

VulkanCommand::VulkanCommand(
    VulkanContext *context,
    VulkanPipeline *pipeline,
    VkCommandPool pool,
    VkDescriptorSet descriptorSet)
    : context(context),
      pipeline(pipeline),
      pool(pool),
      descriptorSet(descriptorSet),
      buffer()
{
}

VulkanCommand::~VulkanCommand()
{
}

void VulkanCommand::Create()
{
    VkCommandBufferAllocateInfo commandBufferInfo{};
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferInfo.commandPool = pool;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(context->GetLogicalDevice(), &commandBufferInfo, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command buffer");
    }
}

void VulkanCommand::Destroy()
{
    vkFreeCommandBuffers(context->GetLogicalDevice(), pool, 1, &buffer);
}

VkCommandBuffer VulkanCommand::BeginCommandBuffer(VkFramebuffer frameBuffer)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    if (vkBeginCommandBuffer(buffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin command buffer");
    }

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(context->GetSwapChainExtent().width);
    viewport.height = static_cast<float>(context->GetSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = context->GetSwapChainExtent();
    vkCmdSetScissor(buffer, 0, 1, &scissor);

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = context->GetRenderPass();
    renderPassBeginInfo.framebuffer = frameBuffer;
    renderPassBeginInfo.renderArea.offset = { 0, 0 };
    renderPassBeginInfo.renderArea.extent = context->GetSwapChainExtent();

    // TODO: add depth stencil value as well
    VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(buffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    return buffer;
}

void VulkanCommand::EndCommandBuffer()
{
    vkCmdEndRenderPass(buffer);

    if (vkEndCommandBuffer(buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to end the command buffer");
    }
}

void VulkanCommand::BindDescriptorSets()
{
    vkCmdBindDescriptorSets(
        buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);
}

void VulkanCommand::BindPipeline()
{
    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());
}

void VulkanCommand::UpdateDescriptor(uint32_t binding, VulkanBuffer *dataBuffer, uint32_t size)
{
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = dataBuffer->GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = static_cast<VkDeviceSize>(size);

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(context->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanCommand::UpdateDescriptor(uint32_t binding, VulkanImage *dataImage)
{
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageView = dataImage->GetImageView();
    imageInfo.sampler = dataImage->GetSampler();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(context->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
}

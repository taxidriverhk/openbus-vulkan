#include "Engine/Vulkan/VulkanCommon.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/VulkanPipeline.h"
#include "Engine/Vulkan/VulkanRenderPass.h"
#include "Engine/Vulkan/Buffer/VulkanBuffer.h"
#include "Engine/Vulkan/Image/VulkanImage.h"
#include "VulkanCommand.h"

VulkanCommand::VulkanCommand(
    VulkanContext *context,
    VulkanRenderPass *renderPass,
    VkCommandPool pool)
    : context(context),
      renderPass(renderPass),
      pool(pool),
      dataUpdated(true),
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
    ASSERT_VK_RESULT_SUCCESS(
        vkAllocateCommandBuffers(context->GetLogicalDevice(), &commandBufferInfo, &buffer),
        "Failed to create command buffer");
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

    ASSERT_VK_RESULT_SUCCESS(
        vkBeginCommandBuffer(buffer, &beginInfo),
        "Failed to begin command buffer");

    // Need to flip the horizontal view port to compensate with the mirrored viewport
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
    renderPassBeginInfo.renderPass = renderPass->GetRenderPass();
    renderPassBeginInfo.framebuffer = frameBuffer;
    renderPassBeginInfo.renderArea.offset = { 0, 0 };
    renderPassBeginInfo.renderArea.extent = context->GetSwapChainExtent();

    VkClearValue clearValues[2];
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(buffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    return buffer;
}

void VulkanCommand::EndCommandBuffer()
{
    vkCmdEndRenderPass(buffer);
    ASSERT_VK_RESULT_SUCCESS(vkEndCommandBuffer(buffer), "Failed to end the command buffer");
}

void VulkanCommand::BindPipeline(VulkanPipeline *pipeline)
{
    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());
}

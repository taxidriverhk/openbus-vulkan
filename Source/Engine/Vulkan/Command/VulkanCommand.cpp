#include "Engine/Vulkan/VulkanCommon.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "VulkanCommand.h"

VulkanCommand::VulkanCommand(VulkanContext *context, VkCommandPool pool)
    : context(context),
      pool(pool),
      buffer()
{
}

VulkanCommand::~VulkanCommand()
{
}

void VulkanCommand::Create(VkCommandBufferLevel level)
{
    VkCommandBufferAllocateInfo commandBufferInfo{};
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferInfo.commandPool = pool;
    commandBufferInfo.level = level;
    commandBufferInfo.commandBufferCount = 1;
    ASSERT_VK_RESULT_SUCCESS(
        vkAllocateCommandBuffers(context->GetLogicalDevice(), &commandBufferInfo, &buffer),
        "Failed to create command buffer");
}

void VulkanCommand::Destroy()
{
    vkFreeCommandBuffers(context->GetLogicalDevice(), pool, 1, &buffer);
}

void VulkanCommand::Reset()
{
    ASSERT_VK_RESULT_SUCCESS(
        vkResetCommandBuffer(buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT),
        "Failed to reset command buffer");
}

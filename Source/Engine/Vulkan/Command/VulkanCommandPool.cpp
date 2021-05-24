#include "Engine/Vulkan/VulkanCommon.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "VulkanCommandPool.h"

VulkanCommandPool::VulkanCommandPool(VulkanContext *context)
    : context(context)
{
}

VulkanCommandPool::~VulkanCommandPool()
{
}

void VulkanCommandPool::Create()
{
    VkCommandPool commandPool = GetOrCreateCommandPool(std::this_thread::get_id());
}

void VulkanCommandPool::Destroy()
{
    for (auto &[threadId, commandPool] : commandPools)
    {
        vkDestroyCommandPool(context->GetLogicalDevice(), commandPool, nullptr);
    }
    commandPools.clear();
}

VkCommandPool VulkanCommandPool::GetOrCreateCommandPool(std::thread::id threadId)
{
    if (commandPools.count(threadId) > 0)
    {
        return commandPools[threadId];
    }

    VkCommandPool commandPool;
    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = context->GetGraphicsQueueIndex();
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_VK_RESULT_SUCCESS(
        vkCreateCommandPool(context->GetLogicalDevice(), &commandPoolInfo, nullptr, &commandPool),
        "Failed to create command pool");
    commandPools.insert(std::make_pair(threadId, commandPool));

    return commandPools[threadId];
}

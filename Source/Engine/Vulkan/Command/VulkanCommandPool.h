#pragma once

#include <thread>
#include <unordered_map>

#include <vulkan/vulkan.h>

class VulkanContext;

class VulkanCommandPool
{
public:
    VulkanCommandPool(VulkanContext *context);
    ~VulkanCommandPool();

    void Create();
    void Destroy();

    VkCommandPool GetOrCreateCommandPool(std::thread::id threadId);

private:
    VulkanContext *context;
    std::unordered_map<std::thread::id, VkCommandPool> commandPools;
};

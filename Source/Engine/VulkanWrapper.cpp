#include "VulkanWrapper.h"

namespace Vulkan
{
    void CreateInstance(VkInstance *vulkanInstance)
    {
        VkApplicationInfo vulkanApplicationInfo{};
        vulkanApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        vulkanApplicationInfo.pApplicationName = APP_NAME;
        vulkanApplicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        vulkanApplicationInfo.pEngineName = "No Engine";
        vulkanApplicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        vulkanApplicationInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo vulkanCreateInfo{};
        vulkanCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        vulkanCreateInfo.pApplicationInfo = &vulkanApplicationInfo;

        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        vulkanCreateInfo.enabledExtensionCount = glfwExtensionCount;
        vulkanCreateInfo.ppEnabledExtensionNames = glfwExtensions;
        vulkanCreateInfo.enabledLayerCount = 0;

        vkCreateInstance(&vulkanCreateInfo, nullptr, vulkanInstance);
    }

    void CreatePipeline()
    {
    }

    void DestroyInstance(VkInstance vulkanInstance)
    {
        vkDestroyInstance(vulkanInstance, nullptr);
    }
}

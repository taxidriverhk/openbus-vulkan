#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "Common/Constants.h"
#include "Engine/Screen.h"

#define VULKAN_VALIDATION_LAYER "VK_LAYER_KHRONOS_validation"

class VulkanContext
{
public:
    VulkanContext(Screen *screen, const bool &enableDebugging);
    ~VulkanContext();

    void Create();
    void Destroy();
    void RecreateSwapChain();
    void WaitIdle();

    Screen * GetScreen() const { return screen; }
    VkQueue GetGraphicsQueue() const { return graphicsQueue; }
    uint32_t GetGraphicsQueueIndex() const { return graphicsQueueIndex; }
    VkQueue GetPresentQueue() const { return presentQueue; }
    uint32_t GetPresentQueueIndex() const { return presentQueueIndex; }

    VkDevice GetLogicalDevice() const { return logicalDevice; }
    VkRenderPass GetRenderPass() const { return renderPass; }
    VkSwapchainKHR GetSwapChain() const { return swapChain; }
    VkExtent2D GetSwapChainExtent() const { return swapChainExtent; }
    std::vector<VkImage> GetSwapChainImages() const { return swapChainImages; }
    std::vector<VkImageView> GetSwapChainImageViews() const { return swapChainImageViews; }

private:
    // Functions required for setting up the Vulkan pipeline
    void CreateImageViews();
    void CreateInstance();
    void CreateLogicalDevice();
    void CreateRenderPass();
    void CreateSwapChain();
    void CreateWindowSurface();

    void DestroyImageViews();
    void DestroySwapChain();

    void EnableDebugging();
    void FindGraphicsAndPresentQueues();
    void FindPhysicalDevice();
    bool TryFindSwapChainDetail(
        VkSurfaceCapabilitiesKHR &capabilities,
        std::vector<VkSurfaceFormatKHR> &formats,
        std::vector<VkPresentModeKHR> &presentModes);
    bool TryFindQueueFamilyIndices(uint32_t &graphicsQueueFamilyIndex, uint32_t &presentQueueFamilyIndex);

    VkInstance instance;
    VkSurfaceKHR surface;
    VkDevice logicalDevice;
    VkPhysicalDevice physicalDevice;

    bool enableDebugging;
    VkDebugUtilsMessengerEXT debugMessenger;

    uint32_t graphicsQueueIndex;
    VkQueue graphicsQueue;
    uint32_t presentQueueIndex;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VkRenderPass renderPass;

    Screen *screen;
};

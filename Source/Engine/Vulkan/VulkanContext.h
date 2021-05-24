#pragma once

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <vulkan/vulkan.h>

#include "Common/Constants.h"
#include "Engine/Screen.h"

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

    VkInstance GetInstance() const { return instance; }
    VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice; }
    VkDevice GetLogicalDevice() const { return logicalDevice; }

    VkFormat GetDepthImageFormat() const { return depthImageFormat; }

    VkSwapchainKHR GetSwapChain() const { return swapChain; }
    VkExtent2D GetSwapChainExtent() const { return swapChainExtent; }
    VkFormat GetSwapChainImageFormat() const { return swapChainImageFormat; }
    std::vector<VkImage> GetSwapChainImages() const { return swapChainImages; }
    std::vector<VkImageView> GetSwapChainImageViews() const { return swapChainImageViews; }

    // TODO: may use a config struct
    VkSampleCountFlagBits GetMSAASampleBits() const { return msaaSamples; }

private:
    static constexpr char * VULKAN_VALIDATION_LAYER = "VK_LAYER_KHRONOS_validation";

    void CreateImageForFrameBuffer(
        VkFormat format,
        VkImageUsageFlags usage,
        VkImageAspectFlags aspect,
        VkImage &image,
        VkImageView &imageView,
        VkDeviceMemory &deviceMemory);

    void CreateSwapChainImageViews();
    void CreateInstance();
    void CreateLogicalDevice();
    void CreateSwapChain();
    void CreateWindowSurface();

    void DestroySwapChainImageViews();
    void DestroyOldSwapChain();
    void DestroySwapChain();

    void EnableDebugging();
    void FindDepthImageFormat();
    void FindGraphicsAndPresentQueues();
    void FindPhysicalDevice();
    VkSampleCountFlagBits GetMaxSampleCount();
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
    VkSwapchainKHR oldSwapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VkImage colorImage;
    VkImageView colorImageView;
    VkDeviceMemory colorImageMemory;
    VkImage depthImage;
    VkImageView depthImageView;
    VkFormat depthImageFormat;
    VkDeviceMemory depthImageMemory;

    VkSampleCountFlagBits msaaSamples;

    Screen *screen;
};

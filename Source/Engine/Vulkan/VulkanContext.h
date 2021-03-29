#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "Common/Constants.h"

class VulkanContext
{
public:
    VulkanContext(GLFWwindow *window, const bool &enableDebugging);
    ~VulkanContext();

    void Create();
    void Destroy();

    VkDevice GetLogicalDevice() const { return logicalDevice; };

private:
    // Functions required for setting up the Vulkan pipeline
    void CreateImageViews();
    void CreateInstance();
    void CreateLogicalDevice();
    void CreateSwapChain();
    void CreateWindowSurface();
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

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    GLFWwindow *window;
};

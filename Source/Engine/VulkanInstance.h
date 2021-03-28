#pragma once

#include "Common/Constants.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

class VulkanInstance
{
public:
    VulkanInstance(GLFWwindow *window, const bool &enableDebugging);
    ~VulkanInstance();

    void Create();
    void Destroy();

private:
    void CreateImageViews();
    void CreateInstance();
    void CreateLogicalDevice();
    void CreatePipeline();
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

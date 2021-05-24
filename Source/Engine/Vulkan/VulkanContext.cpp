#include "Common/Logger.h"
#include "VulkanCommon.h"
#include "VulkanContext.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData);

VulkanContext::VulkanContext(Screen *screen, const bool &enableDebugging)
    : instance(),
    debugMessenger(),
    surface(),
    logicalDevice(),
    physicalDevice(),
    graphicsQueueIndex(),
    graphicsQueue(),
    presentQueueIndex(),
    presentQueue(),
    oldSwapChain(),
    msaaSamples(VK_SAMPLE_COUNT_1_BIT),
    swapChain(),
    swapChainImageFormat(VK_FORMAT_UNDEFINED),
    swapChainExtent(),
    swapChainImages(),
    swapChainImageViews(),
    colorImage(),
    colorImageView(),
    colorImageMemory(),
    depthImage(),
    depthImageView(),
    depthImageFormat(),
    depthImageMemory(),
    screen(screen),
    enableDebugging(enableDebugging)
{
}

VulkanContext::~VulkanContext()
{
}

void VulkanContext::Create()
{
    CreateInstance();
    CreateWindowSurface();

    if (enableDebugging)
    {
        EnableDebugging();
    }

    FindPhysicalDevice();
    CreateLogicalDevice();
    FindGraphicsAndPresentQueues();
    CreateSwapChain();
    CreateSwapChainImageViews();
    FindDepthImageFormat();
}

void VulkanContext::Destroy()
{
    DestroySwapChainImageViews();
    DestroySwapChain();
    
    vkDestroyDevice(logicalDevice, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);

    if (enableDebugging)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugFunc = (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        destroyDebugFunc(instance, debugMessenger, nullptr);
    }

    vkDestroyInstance(instance, nullptr);
}

void VulkanContext::RecreateSwapChain()
{
    oldSwapChain = swapChain;
    DestroySwapChainImageViews();
    CreateSwapChain();
    DestroyOldSwapChain();
    CreateSwapChainImageViews();
}

void VulkanContext::WaitIdle()
{
    vkDeviceWaitIdle(logicalDevice);
}

void VulkanContext::CreateImageForFrameBuffer(
    VkFormat format,
    VkImageUsageFlags usage,
    VkImageAspectFlags aspect,
    VkImage &image,
    VkImageView &imageView,
    VkDeviceMemory &deviceMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = swapChainExtent.width;
    imageInfo.extent.height = swapChainExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = msaaSamples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ASSERT_VK_RESULT_SUCCESS(
        vkCreateImage(logicalDevice, &imageInfo, nullptr, &image),
        "Failed to create image");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

    uint32_t imageMemoryTypeIndex = 0;
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memRequirements.memoryTypeBits & (1 << i))
            && (memProperties.memoryTypes[i].propertyFlags
                & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            imageMemoryTypeIndex = i;
            break;
        }
    }

    VkMemoryAllocateInfo memoryAllocInfo{};
    memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocInfo.allocationSize = memRequirements.size;
    memoryAllocInfo.memoryTypeIndex = imageMemoryTypeIndex;
    ASSERT_VK_RESULT_SUCCESS(
        vkAllocateMemory(logicalDevice, &memoryAllocInfo, nullptr, &deviceMemory),
        "Failed to allocate image memory");

    vkBindImageMemory(logicalDevice, image, deviceMemory, 0);

    VkImageViewCreateInfo imageViewInfo{};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = image;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = format;
    imageViewInfo.subresourceRange.aspectMask = aspect;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;
    ASSERT_VK_RESULT_SUCCESS(
        vkCreateImageView(logicalDevice, &imageViewInfo, nullptr, &imageView),
        "Failed to create image view");
}

void VulkanContext::CreateSwapChainImageViews()
{
    uint32_t swapChainImageCount;
    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapChainImageCount, nullptr);
    swapChainImages.resize(swapChainImageCount);
    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapChainImageCount, swapChainImages.data());

    for (const VkImage &swapChainImage : swapChainImages)
    {
        VkImageView swapChainImageView;
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImage;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        ASSERT_VK_RESULT_SUCCESS(
            vkCreateImageView(logicalDevice, &createInfo, nullptr, &swapChainImageView),
            "Failed to create image views");
        swapChainImageViews.push_back(swapChainImageView);
    }
}

void VulkanContext::CreateInstance()
{
    VkApplicationInfo vulkanApplicationInfo{};
    vulkanApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vulkanApplicationInfo.pApplicationName = APP_NAME;
    vulkanApplicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    vulkanApplicationInfo.pEngineName = "OpenBus";
    vulkanApplicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    vulkanApplicationInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo vulkanCreateInfo{};
    vulkanCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vulkanCreateInfo.pApplicationInfo = &vulkanApplicationInfo;

    std::vector<const char *> sfmlExtensions = sf::Vulkan::getGraphicsRequiredInstanceExtensions();
    std::vector<const char *> extensions(sfmlExtensions);

    const char *enabledLayerNames[] = { VULKAN_VALIDATION_LAYER };
    if (enableDebugging)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        vulkanCreateInfo.enabledLayerCount = 1;
        vulkanCreateInfo.ppEnabledLayerNames = enabledLayerNames;
    }
    else
    {
        vulkanCreateInfo.enabledLayerCount = 0;
    }

    vulkanCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    vulkanCreateInfo.ppEnabledExtensionNames = extensions.data();
    
    ASSERT_VK_RESULT_SUCCESS(
        vkCreateInstance(&vulkanCreateInfo, nullptr, &instance),
        "Failed to create Vulkan instance");
}

void VulkanContext::CreateLogicalDevice()
{
    uint32_t graphicsQueueFamilyIndex, presentQueueFamilyIndex;
    TryFindQueueFamilyIndices(graphicsQueueFamilyIndex, presentQueueFamilyIndex);

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures physicalDeviceFeatures{};
    physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo logicalDeviceCreateInfo{};
    logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    logicalDeviceCreateInfo.queueCreateInfoCount = 1;
    logicalDeviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

    char *enabledExtensionNames[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    logicalDeviceCreateInfo.enabledExtensionCount = 1;
    logicalDeviceCreateInfo.ppEnabledExtensionNames = enabledExtensionNames;

    const char *enabledLayerNames[] = { VULKAN_VALIDATION_LAYER };
    if (enableDebugging)
    {
        logicalDeviceCreateInfo.enabledLayerCount = 1;
        logicalDeviceCreateInfo.ppEnabledLayerNames = enabledLayerNames;
    }
    else
    {
        logicalDeviceCreateInfo.enabledLayerCount = 0;
    }

    ASSERT_VK_RESULT_SUCCESS(
        vkCreateDevice(physicalDevice, &logicalDeviceCreateInfo, nullptr, &logicalDevice),
        "Failed to create logical device");
}

void VulkanContext::CreateSwapChain()
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> availableFormats;
    std::vector<VkPresentModeKHR> availablePresentModes;
    if (!TryFindSwapChainDetail(capabilities, availableFormats, availablePresentModes))
    {
        throw std::runtime_error("Failed to find swap chain detail");
    }

    VkSurfaceFormatKHR chosenFormat = availableFormats.at(0);
    for (const VkSurfaceFormatKHR &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            chosenFormat = availableFormat;
            break;
        }
    }
    swapChainImageFormat = chosenFormat.format;

    VkPresentModeKHR chosenPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const VkPresentModeKHR &presentMode : availablePresentModes)
    {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            chosenPresentMode = presentMode;
            break;
        }
    }

    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        swapChainExtent = capabilities.currentExtent;
    }
    else
    {
        uint32_t screenWidthInPixels = std::clamp<uint32_t>(
            screen->GetWidth(),
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width);
        uint32_t screenHeightInPixels = std::clamp<uint32_t>(
            screen->GetHeight(),
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height);
        swapChainExtent = VkExtent2D{
            screenWidthInPixels,
            screenHeightInPixels
        };
        swapChainExtent.width = std::max(capabilities.minImageExtent.width,
            std::min(capabilities.maxImageExtent.width, swapChainExtent.width));
        swapChainExtent.height = std::max(capabilities.minImageExtent.height,
            std::min(capabilities.maxImageExtent.height, swapChainExtent.height));
    }

    uint32_t imageCount = std::clamp<uint32_t>(2, capabilities.minImageCount, capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = chosenFormat.format;
    createInfo.imageColorSpace = chosenFormat.colorSpace;
    createInfo.imageExtent = swapChainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t graphicsQueueFamilyIndex, presentQueueFamilyIndex;
    TryFindQueueFamilyIndices(graphicsQueueFamilyIndex, presentQueueFamilyIndex);
    if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
    {
        uint32_t queueFamilyIndices[] = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = chosenPresentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapChain;

    ASSERT_VK_RESULT_SUCCESS(
        vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain),
        "Failed to create swap chain");
}

void VulkanContext::CreateWindowSurface()
{
    if (!screen->GetWindow()->createVulkanSurface(instance, surface))
    {
        throw std::runtime_error("Failed to create SFML surface");
    }
}

void VulkanContext::DestroySwapChainImageViews()
{
    for (VkImageView swapChainImageView : swapChainImageViews)
    {
        vkDestroyImageView(logicalDevice, swapChainImageView, nullptr);
    }
    swapChainImageViews.clear();
}

void VulkanContext::DestroyOldSwapChain()
{
    if (oldSwapChain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(logicalDevice, oldSwapChain, nullptr);
        oldSwapChain = VK_NULL_HANDLE;
    }
}

void VulkanContext::DestroySwapChain()
{
    vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
}

void VulkanContext::EnableDebugging()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;

    PFN_vkCreateDebugUtilsMessengerEXT createDebugFunc = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    ASSERT_VK_RESULT_SUCCESS(
        createDebugFunc(instance, &createInfo, nullptr, &debugMessenger),
        "Failed to create debug function");
}

void VulkanContext::FindDepthImageFormat()
{
    for (VkFormat format : DEPTH_IMAGE_FORMATS)
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
        if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            == VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            depthImageFormat = format;
            return;
        }
    }

    throw std::runtime_error("Failed to find suitable depth image format");
}

void VulkanContext::FindGraphicsAndPresentQueues()
{
    uint32_t graphicsQueueFamilyIndex, presentQueueFamilyIndex;
    TryFindQueueFamilyIndices(graphicsQueueFamilyIndex, presentQueueFamilyIndex);

    graphicsQueueIndex = graphicsQueueFamilyIndex;
    presentQueueIndex = presentQueueFamilyIndex;
    vkGetDeviceQueue(logicalDevice, graphicsQueueFamilyIndex, 0, &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, presentQueueFamilyIndex, 0, &presentQueue);
}

void VulkanContext::FindPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("Failed to find device/GPU with Vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    std::vector<VkPhysicalDevice>::iterator deviceIterator;
    for (deviceIterator = devices.begin(); deviceIterator < devices.end(); deviceIterator++)
    {
        physicalDevice = *deviceIterator;

        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;

        vkGetPhysicalDeviceProperties(*deviceIterator, &deviceProperties);
        vkGetPhysicalDeviceFeatures(*deviceIterator, &deviceFeatures);

        uint32_t graphicsQueueFamilyIndex, presentQueueFamilyIndex;
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
        bool queueFamiliesFound = TryFindQueueFamilyIndices(graphicsQueueFamilyIndex, presentQueueFamilyIndex);
        bool swapChainSupported = TryFindSwapChainDetail(capabilities, formats, presentModes);
        if (queueFamiliesFound && swapChainSupported)
        {
            msaaSamples = GetMaxSampleCount();
            return;
        }
    }

    physicalDevice = VK_NULL_HANDLE;
    throw std::runtime_error("Failed to find suitable phyiscal device/GPU");
}

VkSampleCountFlagBits VulkanContext::GetMaxSampleCount()
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlagBits maximumBit = VK_SAMPLE_COUNT_4_BIT;
    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts
        & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    VkSampleCountFlagBits possibleBits[] =
    {
        VK_SAMPLE_COUNT_64_BIT,
        VK_SAMPLE_COUNT_32_BIT,
        VK_SAMPLE_COUNT_16_BIT,
        VK_SAMPLE_COUNT_8_BIT,
        VK_SAMPLE_COUNT_4_BIT,
        VK_SAMPLE_COUNT_2_BIT
    };
    for (uint32_t i = 0; i < 6; i++)
    {
        if ((counts & possibleBits[i]) && possibleBits[i] == maximumBit)
        {
            return possibleBits[i];
        }
    }
    return VK_SAMPLE_COUNT_1_BIT;
}

bool VulkanContext::TryFindSwapChainDetail(
    VkSurfaceCapabilitiesKHR &capabilities,
    std::vector<VkSurfaceFormatKHR> &formats,
    std::vector<VkPresentModeKHR> &presentModes)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    bool requiredExtensionFound = false;
    for (const VkExtensionProperties &extension : availableExtensions)
    {
        if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
        {
            requiredExtensionFound = true;
            break;
        }
    }
    if (!requiredExtensionFound)
    {
        return false;
    }

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    if (formatCount > 0)
    {
        formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    if (presentModeCount > 0)
    {
        presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &formatCount, presentModes.data());
    }

    return formatCount > 0 && presentModeCount > 0;
}

bool VulkanContext::TryFindQueueFamilyIndices(uint32_t &graphicsQueueFamilyIndex, uint32_t &presentQueueFamilyIndex)
{
    graphicsQueueFamilyIndex = UINT32_MAX;
    presentQueueFamilyIndex = UINT32_MAX;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    uint32_t currentIndex = 0;
    for (; currentIndex < queueFamilies.size(); currentIndex++)
    {
        VkQueueFamilyProperties queueFamily = queueFamilies.at(currentIndex);
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphicsQueueFamilyIndex = currentIndex;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, currentIndex, surface, &presentSupport);
        if (presentSupport)
        {
            presentQueueFamilyIndex = currentIndex;
        }

        if (graphicsQueueFamilyIndex != UINT32_MAX && presentQueueFamilyIndex != UINT32_MAX)
        {
            return true;
        }
    }

    return false;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    std::string message = "Vulkan validation: " + std::string(pCallbackData->pMessage);
    Logger::Log(LogLevel::Debug, message);
    return VK_FALSE;
}

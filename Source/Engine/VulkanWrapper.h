#pragma once

#include "Common/Constants.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Vulkan
{
    void CreateInstance(VkInstance* vulkanInstance);
    void CreatePipeline();
    void DestroyInstance(VkInstance vulkanInstance);
}

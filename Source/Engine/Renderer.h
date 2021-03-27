#pragma once

#include "VulkanWrapper.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void Cleanup();
    void DrawScene();
    void PrepareContext();
private:
    
    VkInstance vulkanInstance;
};

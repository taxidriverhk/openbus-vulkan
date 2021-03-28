#pragma once

#include "VulkanInstance.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void Cleanup();
    void DrawScene();
    void PrepareContext(GLFWwindow *window);

private:
    std::unique_ptr<VulkanInstance> vulkanInstance;
};

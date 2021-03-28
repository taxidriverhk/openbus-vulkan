#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

class VulkanContext;

class VulkanShader
{
public:
    VulkanShader(
        VulkanContext *context,
        const std::string &shaderCodePath);
    ~VulkanShader();

    bool Compile();
    void Load();
    void Unload();

private:
    std::string shaderCodePath;

    VulkanContext *context;
    VkShaderModule shaderModule;
};

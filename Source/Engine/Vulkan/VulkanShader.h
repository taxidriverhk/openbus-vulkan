#pragma once

#define GLFW_INCLUDE_VULKAN

#include <fstream>
#include <iterator>
#include <string>
#include <GLFW/glfw3.h>
#include <shaderc/shaderc.hpp>

#include "VulkanContext.h"

class VulkanShader
{
public:
    VulkanShader(VulkanContext *context);
    ~VulkanShader();

    bool Compile(const std::string &shaderCodePath);
    void Load();
    void Unload();

private:
    std::string shaderCodePath;
    std::vector<uint32_t> compiledShaderBytes;
    shaderc::Compiler compiler;

    VulkanContext *context;
    VkShaderModule shaderModule;
};

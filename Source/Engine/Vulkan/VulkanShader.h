#pragma once

#define GLFW_INCLUDE_VULKAN

#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <GLFW/glfw3.h>
#include <shaderc/shaderc.hpp>

#include "VulkanContext.h"

enum class VulkanShaderType
{
    VERTEX,
    FRAGMENT
};

class VulkanShader
{
public:
    VulkanShader(VulkanContext *context, VulkanShaderType type);
    ~VulkanShader();

    VkShaderModule GetShaderModule() const { return shaderModule; }

    bool Compile(const std::string &shaderCodePath);
    void Load();
    void Unload();

private:
    VulkanShaderType type;
    std::string shaderCodePath;
    std::vector<uint32_t> compiledShaderBytes;
    shaderc::Compiler compiler;

    VulkanContext *context;
    VkShaderModule shaderModule;

    static std::map<VulkanShaderType, shaderc_shader_kind> shaderTypeToKindMap;
};

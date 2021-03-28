#include "VulkanShader.h"

VulkanShader::VulkanShader(
    VulkanContext *context,
    const std::string &shaderCodePath)
    : context(context),
      shaderCodePath(shaderCodePath),
      shaderModule()
{
}

VulkanShader::~VulkanShader()
{
}

bool VulkanShader::Compile()
{
    return false;
}

void VulkanShader::Load()
{
}

void VulkanShader::Unload()
{
}

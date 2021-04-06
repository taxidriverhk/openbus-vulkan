#include "VulkanCommon.h"
#include "VulkanShader.h"

std::map<VulkanShaderType, shaderc_shader_kind> VulkanShader::shaderTypeToKindMap =
{
    { VulkanShaderType::Vertex, shaderc_shader_kind::shaderc_glsl_vertex_shader },
    { VulkanShaderType::Fragment, shaderc_shader_kind::shaderc_glsl_fragment_shader }
};

VulkanShader::VulkanShader(VulkanContext *context, VulkanShaderType type)
    : context(context),
      compiler(),
      compiledShaderBytes(std::vector<uint32_t>()),
      shaderModule(),
      type(type)
{
}

VulkanShader::~VulkanShader()
{
}

bool VulkanShader::Compile(const std::string &shaderCodePath)
{
    std::ifstream sourceCodeFileStream(shaderCodePath);
    if (!sourceCodeFileStream.is_open())
    {
        return false;
    }

    std::string sourceCode(
        (std::istreambuf_iterator<char>(sourceCodeFileStream)),
        std::istreambuf_iterator<char>());

    shaderc::CompileOptions options;
    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
        sourceCode,
        shaderTypeToKindMap[type],
        shaderCodePath.c_str(),
        options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        return false;
    }

    compiledShaderBytes.assign(result.cbegin(), result.cend());
    return true;
}

void VulkanShader::Load()
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = compiledShaderBytes.size() * sizeof(uint32_t);
    createInfo.pCode = compiledShaderBytes.data();

    ASSERT_VK_RESULT_SUCCESS(
        vkCreateShaderModule(context->GetLogicalDevice(), &createInfo, nullptr, &shaderModule),
        "Failed to load shader module");
}

void VulkanShader::Unload()
{
    vkDestroyShaderModule(context->GetLogicalDevice(), shaderModule, nullptr);
}

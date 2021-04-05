#pragma once

#include <stdexcept>

#include <vulkan/vulkan.h>

#include "Engine/Mesh.h"

static constexpr char *SHADER_MAIN_FUNCTION_NAME = "main";

static constexpr int STATIC_PIPELINE_DESCRIPTOR_POOL_SIZE_COUNT = 2;
static constexpr VkDescriptorPoolSize STATIC_PIPELINE_DESCRIPTOR_POOL_SIZES[] =
{
    {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          // type
        3                                           // maximum count
    },
    {
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        3 * 3                                       // reserved for diffuse, normal
                                                    // and specular for each frame buffer
    }
};

static constexpr int STATIC_PIPELINE_DESCRIPTOR_LAYOUT_BINDING_COUNT = 2;
static constexpr VkDescriptorSetLayoutBinding STATIC_PIPELINE_DESCRIPTOR_LAYOUT_BINDINGS[] =
{
    // Uniform buffer
    {
        0,                                          // binding
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          // descriptorType
        1,                                          // descriptorCount
        VK_SHADER_STAGE_VERTEX_BIT,                 // stageFlags
        nullptr                                     // pImmutableSamplers
    },
    // Texture sampler (diffuse only for now)
    {
        1,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        1,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr
    }
};

static constexpr int VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_COUNT = 3;
static constexpr VkVertexInputAttributeDescription VERTEX_INPUT_ATTRIBUTE_DESCRIPTIONS[] =
{
    {
        0,                                          // location
        0,                                          // binding
        VK_FORMAT_R32G32_SFLOAT,                    // format
        offsetof(Vertex, position)                  // offset
    },
    {
        1,
        0,
        VK_FORMAT_R32G32B32_SFLOAT,
        offsetof(Vertex, color)
    },
    {
        2,
        0,
        VK_FORMAT_R32G32_SFLOAT,
        offsetof(Vertex, uv)
    }
};

static void AssertVulkanResult(VkResult result, std::string error)
{
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Vulkan API call error: " + error);
    }
}

static void AssertVulkanResult(VkResult result)
{
    AssertVulkanResult(result, "error code -" + std::to_string(result));
}

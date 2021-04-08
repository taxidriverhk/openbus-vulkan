#pragma once

#include <stdexcept>

#include <vulkan/vulkan.h>

#include "Engine/Mesh.h"

#define ASSERT_VK_RESULT_SUCCESS(result, error)                                                   \
    if (result != VK_SUCCESS)                                                                     \
    {                                                                                             \
        std::string message = "Vulkan API call error (" + std::to_string(result) + "): " + error; \
        throw std::runtime_error(message);                                                        \
    }                                                                                             \

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
        8192 * 3                                    // reserved for diffuse, normal
                                                    // and specular for each frame buffer
    }
};

static constexpr int STATIC_PIPELINE_UNIFORM_DESCRIPTOR_LAYOUT_BINDING_COUNT = 1;
static constexpr VkDescriptorSetLayoutBinding STATIC_PIPELINE_UNIFORM_DESCRIPTOR_LAYOUT_BINDINGS[] =
{
    // Uniform buffer
    {
        0,                                          // binding
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          // descriptorType
        1,                                          // descriptorCount
        VK_SHADER_STAGE_VERTEX_BIT,                 // stageFlags
        nullptr                                     // pImmutableSamplers
    }
};

static constexpr int STATIC_PIPELINE_PER_OBJECT_DESCRIPTOR_LAYOUT_BINDING_COUNT = 1;
static constexpr VkDescriptorSetLayoutBinding STATIC_PIPELINE_PER_OBJECT_DESCRIPTOR_LAYOUT_BINDINGS[] =
{
    // Texture sampler (diffuse only for now)
    {
        0,
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
        VK_FORMAT_R32G32B32_SFLOAT,                 // format
        offsetof(Vertex, position)                  // offset
    },
    {
        1,
        0,
        VK_FORMAT_R32G32B32_SFLOAT,
        offsetof(Vertex, normal)
    },
    {
        2,
        0,
        VK_FORMAT_R32G32_SFLOAT,
        offsetof(Vertex, uv)
    }
};

static constexpr VkFormat DEPTH_IMAGE_FORMATS[] =
{
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D24_UNORM_S8_UINT
};

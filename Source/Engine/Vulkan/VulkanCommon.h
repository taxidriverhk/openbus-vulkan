#pragma once

#include <stdexcept>

#include <vulkan/vulkan.h>

#include "Engine/Mesh.h"
#include "Engine/Vertex.h"

#define ASSERT_VK_RESULT_SUCCESS(result, error)                                                   \
    if (result != VK_SUCCESS)                                                                     \
    {                                                                                             \
        std::string message = "Vulkan API call error (" + std::to_string(result) + "): " + error; \
        throw std::runtime_error(message);                                                        \
    }                                                                                             \

static constexpr char *SHADER_MAIN_FUNCTION_NAME = "main";

static constexpr char *STATIC_PIPELINE_VERTEX_SHADER = "shaders/static_vertex_shader.glsl";
static constexpr char *STATIC_PIPELINE_FRAGMENT_SHADER = "shaders/static_fragment_shader.glsl";
static constexpr char *CUBEMAP_PIPELINE_VERTEX_SHADER = "shaders/cubemap_vertex_shader.glsl";
static constexpr char *CUBEMAP_PIPELINE_FRAGMENT_SHADER = "shaders/cubemap_fragment_shader.glsl";
static constexpr char *TERRAIN_PIPELINE_VERTEX_SHADER = "shaders/terrain_vertex_shader.glsl";
static constexpr char *TERRAIN_PIPELINE_FRAGMENT_SHADER = "shaders/terrain_fragment_shader.glsl";
static constexpr char *SCREEN_PIPELINE_VERTEX_SHADER = "shaders/screen_vertex_shader.glsl";
static constexpr char *SCREEN_PIPELINE_FRAGMENT_SHADER = "shaders/screen_fragment_shader.glsl";

static constexpr int STATIC_PIPELINE_DESCRIPTOR_POOL_SIZE_COUNT = 2;
static constexpr VkDescriptorPoolSize STATIC_PIPELINE_DESCRIPTOR_POOL_SIZES[] =
{
    {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          // type
        3 + 8192 * 3                                // maximum count
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

static constexpr int STATIC_PIPELINE_IMAGE_DESCRIPTOR_LAYOUT_BINDING_COUNT = 1;
static constexpr VkDescriptorSetLayoutBinding STATIC_PIPELINE_IMAGE_DESCRIPTOR_LAYOUT_BINDINGS[] =
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

static constexpr int STATIC_PIPELINE_INSTANCE_DESCRIPTOR_LAYOUT_BINDING_COUNT = 1;
static constexpr VkDescriptorSetLayoutBinding STATIC_PIPELINE_INSTANCE_DESCRIPTOR_LAYOUT_BINDINGS[] =
{
    // Instance buffer for each object
    {
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_VERTEX_BIT,
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

static constexpr int SCREEN_OBJECT_INPUT_ATTRIBUTE_DESCRIPTION_COUNT = 3;
static constexpr VkVertexInputAttributeDescription SCREEN_OBJECT_INPUT_ATTRIBUTE_DESCRIPTIONS[] =
{
    {
        0,
        0,
        VK_FORMAT_R32G32B32_SFLOAT,
        offsetof(ScreenObjectVertex, color)
    },
    {
        1,
        0,
        VK_FORMAT_R32G32_SFLOAT,
        offsetof(ScreenObjectVertex, position)
    },
    {
        2,
        0,
        VK_FORMAT_R32G32_SFLOAT,
        offsetof(ScreenObjectVertex, uv)
    }
};

static constexpr VkFormat DEPTH_IMAGE_FORMATS[] =
{
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D24_UNORM_S8_UINT
};

class VulkanBuffer;
class VulkanImage;
class VulkanTexture;
class VulkanPipeline;

// Drawing Buffers
struct VulkanCubeMapBuffer
{
    VulkanBuffer *vertexBuffer;
    VulkanBuffer *indexBuffer;
    VulkanTexture *textureBuffer;
};

struct VulkanTerrainBuffer
{
    VulkanBuffer *vertexBuffer;
    VulkanBuffer *indexBuffer;
    VulkanTexture *textureBuffer;
};

struct VulkanScreenObjectBuffer
{
    VulkanBuffer *vertexBuffer;
    VulkanTexture *textureBuffer;
};

struct VulkanEntityBuffer
{
    std::vector<VulkanBuffer *> instanceBuffers;
    VulkanBuffer *vertexBuffer;
    VulkanBuffer *indexBuffer;
    VulkanTexture *textureBuffer;
};

struct VulkanEntityBufferIds
{
    uint32_t instanceBufferId;
    uint32_t vertexBufferId;
    uint32_t indexBufferId;
    uint32_t textureBufferId;
};

struct VulkanDrawingPipelines
{
    VulkanPipeline *staticPipeline;
    VulkanPipeline *cubeMapPipeline;
    VulkanPipeline *terrainPipeline;
    VulkanPipeline *screenPipeline;
};

struct VulkanDrawingBuffer
{
    VulkanBuffer *uniformBuffer;
    VulkanBuffer *screenBuffer;
    VulkanCubeMapBuffer cubeMapBuffer;
    std::vector<VulkanTerrainBuffer> terrainBuffers;
    std::vector<VulkanEntityBuffer> entityBuffers;
    std::vector<VulkanScreenObjectBuffer> screenObjectBuffers;
};

// Buffer input
struct VulkanInstanceBufferInput
{
    glm::mat4 transformation;
};

struct VulkanScreenBufferInput
{
    float screenWidth;
    float screenHeight;
};

struct VulkanUniformBufferInput
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 eyePosition;
    glm::vec3 lightPosition;
};

// Push constants
struct VulkanMeshPushConstant
{
    float fogDensity;
    float fogGradient;
    alignas(16) glm::vec3 fogColor; // To match the GLSL alignment requirement
};

struct VulkanPushConstants
{
    VulkanMeshPushConstant meshPushConstant;
};

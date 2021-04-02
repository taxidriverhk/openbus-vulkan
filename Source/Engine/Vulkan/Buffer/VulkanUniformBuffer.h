#pragma once

#include <glm/glm.hpp>

#include "VulkanBuffer.h"

struct VulkanUniformBufferInput
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

class VulkanUniformBuffer : public VulkanBuffer
{
public:
    VulkanUniformBuffer(
        VulkanContext *context,
        VkCommandPool commandPool);
    ~VulkanUniformBuffer();

    void Load() override;
    uint32_t Size() const override { return sizeof(VulkanUniformBufferInput); }

private:
};

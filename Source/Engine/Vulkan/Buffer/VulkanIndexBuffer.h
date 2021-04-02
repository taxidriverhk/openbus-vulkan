#pragma once

#include "VulkanBuffer.h"

class VulkanIndexBuffer : public VulkanBuffer
{
public:
    VulkanIndexBuffer(
        VulkanContext *context,
        VkCommandPool commandPool,
        std::vector<uint32_t> indices);
    ~VulkanIndexBuffer();

    void Load() override;
    uint32_t Size() const override { return static_cast<uint32_t>(indices.size()); }

private:
    std::vector<uint32_t> indices;
};

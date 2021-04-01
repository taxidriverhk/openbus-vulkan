#pragma once

#include <vector>

#include "VulkanBuffer.h"
#include "Engine/Mesh.h"

class VulkanVertexBuffer : public VulkanBuffer
{
public:
    VulkanVertexBuffer(
        VulkanContext *context,
        VkCommandPool commandPool,
        std::vector<Vertex> vertices);
    ~VulkanVertexBuffer();

    void Load() override;
    uint32_t Size() const override { return static_cast<uint32_t>(vertices.size()); }
    void Unload() override;

private:
    std::vector<Vertex> vertices;
};

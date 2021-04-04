#pragma once

#include <memory>
#include <unordered_map>

#include <vulkan/vulkan.h>

#include "Engine/Vulkan/Buffer/VulkanBuffer.h"
#include "VulkanCommand.h"

class VulkanDefaultRenderCommand : public VulkanCommand
{
public:
    VulkanDefaultRenderCommand(
        VulkanContext *context,
        VulkanPipeline *pipeline,
        VkCommandPool pool,
        VkDescriptorSet descriptorSet,
        std::unordered_map<uint32_t, std::unique_ptr<VulkanBuffer>> &vertexBuffers,
        std::unordered_map<uint32_t, std::unique_ptr<VulkanBuffer>> &indexBuffers,
        VulkanUniformBufferInput &uniformBuffer);
    ~VulkanDefaultRenderCommand();

    void Record(VkFramebuffer frameBuffer) override;
    void SetDataUpdated(bool nextState) { dataUpdated = nextState; }

private:
    bool dataUpdated;

    std::unordered_map<uint32_t, std::unique_ptr<VulkanBuffer>> &vertexBuffers;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanBuffer>> &indexBuffers;
    VulkanUniformBufferInput &uniformBuffer;
};

#pragma once

#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.h>

#include "VulkanCommand.h"

struct VulkanCubeMapBuffer;
struct VulkanTerrainBuffer;
struct VulkanDrawingPipelines;
struct VulkanPushConstants;
class VulkanContext;
class VulkanCommandPool;
class VulkanRenderPass;

class VulkanCommandManager
{
public:
    VulkanCommandManager(
        VulkanContext *context,
        VulkanCommandPool *commandPool,
        VulkanRenderPass *renderPass,
        VulkanDrawingPipelines pipelines);
    ~VulkanCommandManager();

    VkCommandBuffer GetCommandBuffer(uint32_t imageIndex) const
    {
        return primaryCommandBuffers[imageIndex]->GetBuffer();
    }

    void Create(uint32_t frameBufferSize);
    void Destroy();
    void Record(
        uint32_t imageIndex,
        VkFramebuffer framebuffer,
        VulkanPushConstants pushConstants,
        VulkanDrawingBuffer drawingBuffer);
    void Reset(uint32_t previousImageIndex);

private:
    void BindPipeline(VkCommandBuffer commandBuffer, VulkanPipeline *pipeline);
    void PushConstant(
        VkCommandBuffer commandBuffer,
        VkShaderStageFlags stage,
        VulkanPipeline *pipeline,
        void *data,
        uint32_t size);

    // Primary command buffer
    VkCommandBuffer BeginPrimaryCommand(uint32_t imageIndex, VkFramebuffer frameBuffer);
    // Secondary command buffer
    VkCommandBuffer BeginSecondaryCommand(uint32_t imageIndex, VkFramebuffer frameBuffer);
    VulkanCommand * RequestSecondaryCommandBuffer(uint32_t imageIndex);

    void EndCommand(VkCommandBuffer commandBuffer);
    void SetViewPortAndScissor(VkCommandBuffer commandBuffer);

    std::vector<std::unique_ptr<VulkanCommand>> primaryCommandBuffers;
    struct SecondaryCommandBuffer
    {
        int activeBuffersInUse;
        std::vector<std::unique_ptr<VulkanCommand>> commandBuffers;
    };
    // Each thread has number of SecondaryCommandBuffer's equal to the number of frame buffers
    // Then each SecondaryCommandBuffer allows one or more command buffer to be defined
    std::unordered_map<std::thread::id, std::vector<SecondaryCommandBuffer>> secondaryCommandBuffers;

    uint32_t frameBufferSize;

    VulkanContext *context;
    VulkanCommandPool *commandPool;
    VulkanRenderPass *renderPass;
    VulkanDrawingPipelines pipelines;
};

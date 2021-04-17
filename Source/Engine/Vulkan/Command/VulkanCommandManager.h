#pragma once

#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.h>

struct VulkanCubeMapBuffer;
struct VulkanDrawingPipelines;
class VulkanCommand;
class VulkanContext;
class VulkanRenderPass;

class VulkanCommandManager
{
public:
    VulkanCommandManager(
        VulkanContext *context,
        VulkanRenderPass *renderPass,
        VulkanDrawingPipelines pipelines);
    ~VulkanCommandManager();

    VkCommandBuffer GetCommandBuffer(uint32_t imageIndex) const
    {
        return primaryCommandBuffers[imageIndex]->GetBuffer();
    }

    void Create(uint32_t frameBufferSize);
    void Destroy();
    VkCommandPool GetOrCreateCommandPool(std::thread::id threadId);
    void Record(
        uint32_t imageIndex,
        VkFramebuffer framebuffer,
        VulkanBuffer *uniformBuffer,
        VulkanCubeMapBuffer &cubeMapBuffer,
        std::unordered_map<uint32_t, VulkanDrawingCommand> &drawingCommands);
    void TriggerUpdate(uint32_t imageIndex);

private:
    void BindPipeline(VkCommandBuffer commandBuffer, VulkanPipeline *pipeline);

    // Primary command buffer
    VkCommandBuffer BeginPrimaryCommand(uint32_t imageIndex, VkFramebuffer frameBuffer);
    // Secondary command buffer
    VkCommandBuffer BeginSecondaryCommand(uint32_t imageIndex, VkFramebuffer frameBuffer);
    VulkanCommand * RequestSecondaryCommandBuffer(uint32_t imageIndex);
    void ResetSecondaryCommandBuffers();

    void EndCommand(VkCommandBuffer commandBuffer);
    void SetViewPortAndScissor(VkCommandBuffer commandBuffer);

    std::unordered_map<std::thread::id, VkCommandPool> commandPools;
    std::vector<std::unique_ptr<VulkanCommand>> primaryCommandBuffers;
    std::vector<bool> dataUpdated;

    struct SecondaryCommandBufferCache
    {
        std::vector<std::unique_ptr<VulkanCommand>> buffers;
        uint32_t buffersInUse;
    };
    std::unordered_map<std::thread::id, SecondaryCommandBufferCache> secondaryCommandBufferCache;

    uint32_t frameBufferSize;

    VulkanContext *context;
    VulkanRenderPass *renderPass;
    VulkanDrawingPipelines pipelines;
};

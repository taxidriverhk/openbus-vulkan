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
        return commandBuffers[imageIndex]->GetBuffer();
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
    VkCommandBuffer BeginCommand(uint32_t imageIndex, VkFramebuffer frameBuffer);
    void BindPipeline(VkCommandBuffer commandBuffer, VulkanPipeline *pipeline);
    void EndCommand(VkCommandBuffer commandBuffer);

    std::unordered_map<std::thread::id, VkCommandPool> commandPools;
    std::vector<std::unique_ptr<VulkanCommand>> commandBuffers;
    std::vector<bool> dataUpdated;

    uint32_t frameBufferSize;

    VulkanContext *context;
    VulkanRenderPass *renderPass;
    VulkanDrawingPipelines pipelines;
};

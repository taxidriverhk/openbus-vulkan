#pragma once

#include <vulkan/vulkan.h>

#include "Engine/Vulkan/Buffer/VulkanBuffer.h"
#include "VulkanCommand.h"

struct VulkanDrawingCommand
{
    VulkanBuffer *instanceBuffer;
    VulkanBuffer *vertexBuffer;
    VulkanBuffer *indexBuffer;
    VulkanImage *imageBuffer;
};

class VulkanDefaultRenderCommand : public VulkanCommand
{
public:
    VulkanDefaultRenderCommand(
        VulkanContext *context,
        VulkanPipeline *pipeline,
        VkCommandPool pool,
        std::unordered_map<uint32_t, VulkanDrawingCommand> &drawingCommands,
        VulkanBuffer *uniformBuffer);
    ~VulkanDefaultRenderCommand();

    void Record(VkFramebuffer frameBuffer) override;
    void SetDataUpdated(bool nextState) { dataUpdated = nextState; }

private:
    bool dataUpdated;

    std::unordered_map<uint32_t, VulkanDrawingCommand> &drawingCommands;
    VulkanBuffer *uniformBuffer;
};

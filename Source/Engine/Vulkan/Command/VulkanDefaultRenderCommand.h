#pragma once

#include <vulkan/vulkan.h>

#include "Engine/Vulkan/Buffer/VulkanBuffer.h"
#include "Engine/Vulkan/Image/VulkanImage.h"
#include "VulkanCommand.h"

struct VulkanDrawingPipelines;
struct VulkanDrawingCommand;

class VulkanDefaultRenderCommand : public VulkanCommand
{
public:
    VulkanDefaultRenderCommand(
        VulkanContext *context,
        VulkanRenderPass *renderPass,
        VulkanDrawingPipelines pipelines,
        VkCommandPool pool,
        std::unordered_map<uint32_t, VulkanDrawingCommand> &drawingCommands,
        VulkanBuffer *uniformBuffer,
        VulkanImage *cubeMapBuffer);
    ~VulkanDefaultRenderCommand();

    void Record(VkFramebuffer frameBuffer) override;
    void SetDataUpdated(bool nextState) { dataUpdated = nextState; }

private:
    bool dataUpdated;

    VulkanDrawingPipelines pipelines;
    std::unordered_map<uint32_t, VulkanDrawingCommand> &drawingCommands;
    VulkanBuffer *uniformBuffer;
    VulkanImage *cubeMapBuffer;
};

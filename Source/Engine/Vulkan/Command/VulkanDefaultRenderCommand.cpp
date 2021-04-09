#include "Engine/Vulkan/VulkanPipeline.h"
#include "Engine/Vulkan/Image/VulkanImage.h"
#include "VulkanDefaultRenderCommand.h"

VulkanDefaultRenderCommand::VulkanDefaultRenderCommand(
    VulkanContext *context,
    VulkanPipeline *pipeline,
    VkCommandPool pool,
    std::unordered_map<uint32_t, VulkanDrawingCommand> &drawingCommands,
    VulkanBuffer *uniformBuffer)
    : VulkanCommand(context, pipeline, pool),
      dataUpdated(true),
      drawingCommands(drawingCommands),
      uniformBuffer(uniformBuffer)
{
}

VulkanDefaultRenderCommand::~VulkanDefaultRenderCommand()
{
}

void VulkanDefaultRenderCommand::Record(VkFramebuffer frameBuffer)
{
    // Do not re-record the command if no data update was made
    if (!dataUpdated)
    {
        return;
    }

    VkPipelineLayout pipelineLayout = GetPipeline()->GetPipelineLayout();
    VkCommandBuffer commandBuffer = BeginCommandBuffer(frameBuffer);

    for (const auto &[bufferId, drawingCommand] : drawingCommands)
    {
        BindPipeline();

        VulkanBuffer *instanceBuffer = drawingCommand.instanceBuffer;
        VulkanBuffer *vertexBuffer = drawingCommand.vertexBuffer;
        VulkanBuffer *indexBuffer = drawingCommand.indexBuffer;
        VulkanImage *imageBuffer = drawingCommand.imageBuffer;

        // Bind vertex buffer
        VkBuffer vertexBuffers[] = { vertexBuffer->GetBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        // Bind index buffer
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        // Bind uniform descriptor set
        uniformBuffer->BindDescriptorSet(commandBuffer, 0, pipelineLayout);
        // Bind image sampler descriptor set
        imageBuffer->BindDescriptorSet(commandBuffer, 1, pipelineLayout);
        // Bind instance descriptor set
        instanceBuffer->BindDescriptorSet(commandBuffer, 2, pipelineLayout);

        uint32_t indexCount = indexBuffer->Size() / sizeof(uint32_t);
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    }

    EndCommandBuffer();

    dataUpdated = false;
}

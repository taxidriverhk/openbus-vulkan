#include "Engine/Vulkan/VulkanCommon.h"
#include "Engine/Vulkan/VulkanPipeline.h"
#include "Engine/Vulkan/VulkanRenderPass.h"
#include "Engine/Vulkan/Image/VulkanImage.h"
#include "VulkanDefaultRenderCommand.h"

VulkanDefaultRenderCommand::VulkanDefaultRenderCommand(
    VulkanContext *context,
    VulkanRenderPass *renderPass,
    VulkanDrawingPipelines pipelines,
    VkCommandPool pool,
    std::unordered_map<uint32_t, VulkanDrawingCommand> &drawingCommands,
    VulkanCubeMapBuffer cubeMapBuffer,
    VulkanBuffer *uniformBuffer)
    : VulkanCommand(context, renderPass, pool),
      pipelines(pipelines),
      drawingCommands(drawingCommands),
      cubeMapBuffer(cubeMapBuffer),
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

    VulkanPipeline *staticPipeline = pipelines.staticPipeline;
    VulkanPipeline *cubeMapPipeline = pipelines.cubeMapPipeline;
    VkCommandBuffer commandBuffer = BeginCommandBuffer(frameBuffer);

    for (const auto &[bufferId, drawingCommand] : drawingCommands)
    {
        BindPipeline(staticPipeline);

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
        uniformBuffer->BindDescriptorSet(commandBuffer, 0, staticPipeline->GetPipelineLayout());
        // Bind image sampler descriptor set
        imageBuffer->BindDescriptorSet(commandBuffer, 1, staticPipeline->GetPipelineLayout());
        // Bind instance descriptor set
        instanceBuffer->BindDescriptorSet(commandBuffer, 2, staticPipeline->GetPipelineLayout());

        uint32_t indexCount = indexBuffer->Size() / sizeof(uint32_t);
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    }

    VulkanBuffer *cubeMapVertexBuffer = cubeMapBuffer.vertexBuffer;
    VulkanBuffer *cubeMapIndexBuffer = cubeMapBuffer.indexBuffer;
    if (cubeMapVertexBuffer->IsLoaded())
    {
        BindPipeline(cubeMapPipeline);
        // Bind cubemap mesh
        VkDeviceSize offsets[] = { 0 };
        VkBuffer cubeMapVertexBuffers[] = { cubeMapVertexBuffer->GetBuffer() };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, cubeMapVertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, cubeMapIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        // Bind cubemap descriptor set
        cubeMapBuffer.imageBuffer->BindDescriptorSet(commandBuffer, 1, cubeMapPipeline->GetPipelineLayout());
        // Draw the cube map
        uint32_t indexCount = cubeMapIndexBuffer->Size() / sizeof(uint32_t);
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    }

    EndCommandBuffer();

    dataUpdated = false;
}

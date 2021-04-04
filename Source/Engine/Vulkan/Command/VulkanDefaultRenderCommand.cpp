#include "VulkanDefaultRenderCommand.h"

VulkanDefaultRenderCommand::VulkanDefaultRenderCommand(
    VulkanContext *context,
    VulkanPipeline *pipeline,
    VkCommandPool pool,
    VkDescriptorSet descriptorSet,
    std::unordered_map<uint32_t, std::unique_ptr<VulkanBuffer>> &vertexBuffers,
    std::unordered_map<uint32_t, std::unique_ptr<VulkanBuffer>> &indexBuffers,
    VulkanUniformBufferInput &uniformBuffer)
    : VulkanCommand(context, pipeline, pool, descriptorSet),
      dataUpdated(true),
      vertexBuffers(vertexBuffers),
      indexBuffers(indexBuffers),
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

    VkCommandBuffer commandBuffer = BeginCommandBuffer(frameBuffer);

    for (const auto &vertexBufferEntry : vertexBuffers)
    {
        uint32_t bufferId = vertexBufferEntry.first;
        VulkanBuffer *vertexBuffer = vertexBufferEntry.second.get();
        VulkanBuffer *indexBuffer = indexBuffers[bufferId].get();

        // Bind vertex buffer
        VkBuffer vertexBuffers[] = { vertexBuffer->GetBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        // Bind index buffer
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        // Bind uniform buffer
        BindDescriptorSets();

        uint32_t indexCount = indexBuffer->Size() / sizeof(uint32_t);
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    }

    EndCommandBuffer();

    dataUpdated = false;
}

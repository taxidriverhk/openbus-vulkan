#pragma once

#include <memory>

class VulkanContext;
class VulkanPipeline;
class VulkanRenderPass;

class VulkanPipelineManager
{
public:
    VulkanPipelineManager(VulkanContext *context, VulkanRenderPass *renderPass);
    ~VulkanPipelineManager();

    void Create();
    void Destroy();

    VulkanDrawingPipelines GetDrawingPipelines() const;

private:
    VulkanContext *context;
    VulkanRenderPass *renderPass;

    std::unique_ptr<VulkanPipeline> staticPipeline;
    std::unique_ptr<VulkanPipeline> cubeMapPipeline;
    std::unique_ptr<VulkanPipeline> linePipeline;
    std::unique_ptr<VulkanPipeline> terrainPipeline;
    std::unique_ptr<VulkanPipeline> screenPipeline;
};

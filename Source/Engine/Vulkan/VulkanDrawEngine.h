#pragma once

#include <memory>
#include <vector>
#include <unordered_set>

#include "Engine/DrawEngine.h"
#include "Engine/Screen.h"
#include "Engine/Vertex.h"

#include "VulkanContext.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"

class VulkanPipeline;
class VulkanBufferManager;
class VulkanCommandManager;

class VulkanDrawEngine : public DrawEngine
{
public:
    VulkanDrawEngine(Screen *screen, bool enableDebugging);
    ~VulkanDrawEngine();

    void Destroy() override;
    void DrawFrame() override;
    void DrawText(const std::string &text, int x, int y, float scale) override;
    void Initialize() override;
    void LoadCubeMap(CubeMap &cubeMap) override;
    void LoadEntity(Entity &entity) override;
    void LoadTerrain(Terrain &terrain) override;
    void UpdateCamera(Camera *camera) override;
    void UnloadEntity(uint32_t entityId) override;
    void UnloadTerrain(uint32_t terrainId) override;

private:
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    // The game treats Z-axis as the upward axis, while Vulkan/OpenGL treats Y-axis as the upward axis
    // Therefore, a conversion is required.
    // Texture v-coordinate must also be flipped as the y-axis texture viewport in Vulkan is flipped.
    inline static Vertex ConvertToVulkanVertex(const Vertex &input)
    {
        return
        {
            { input.position.x, input.position.z, input.position.y },
            { input.normal.x, input.normal.z, input.normal.y },
            { input.uv.x, 1.0f - input.uv.y }
        };
    }

    inline static glm::vec3 ConvertToVulkanCoordinates(const glm::vec3 &input)
    {
        return { input.x, input.z, -input.y };
    }

    void CreateCommandBuffers();
    void CreateFrameBuffers();
    void CreatePipelines();
    void CreateSynchronizationObjects();
    void ClearDrawingBuffers();
    void DestroyCommandBuffers();
    void DestroyFrameBuffers();
    void DestroyPipelines();
    void DestroySynchronizationObjects();

    void BeginFrame(uint32_t &imageIndex);
    void EndFrame(uint32_t &imageIndex);
    void Submit(uint32_t &imageIndex);

    void RecreateSwapChain();

    void MarkDataAsUpdated();

    bool enableDebugging;

    Screen *screen;
    std::unique_ptr<VulkanContext> context;
    std::unique_ptr<VulkanRenderPass> renderPass;

    std::unique_ptr<VulkanCommandManager> commandManager;

    std::unique_ptr<VulkanPipeline> staticPipeline;
    std::unique_ptr<VulkanPipeline> cubeMapPipeline;
    std::unique_ptr<VulkanPipeline> terrainPipeline;

    std::unordered_set<uint32_t> bufferIds;
    std::unordered_set<uint32_t> terrainBufferIds;
    std::unique_ptr<VulkanBufferManager> bufferManager;

    // Based on number of swap chain images (which is usually 3)
    std::vector<VkFramebuffer> frameBuffers;
    std::vector<VkFence> imagesInFlight;
    std::vector<bool> dataUpdated;

    // Based on the maximum allowed frames in flight
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    // Active frame that is in use by the GPU
    uint32_t currentInFlightFrame;
};

#pragma once

#include <memory>
#include <vector>
#include <unordered_set>

#include "Engine/DrawEngine.h"
#include "Engine/Screen.h"

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
    void Initialize() override;
    void LoadCubeMap(CubeMap &cubeMap) override;
    void LoadIntoBuffer(Entity &entity) override;
    void UpdateCamera(Camera *camera) override;

private:
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    // The game treats Z-axis as the upward axis, while Vulkan/OpenGL treats Y-axis as the upward axis
    // Therefore, a conversion is required.
    // Texture v-coordinate must also be flipped as the y-axis texture viewport in Vulkan is flipped.
    inline static Vertex ConvertToVulkanVertex(const Vertex &input)
    {
        return
        {
            { -input.position.x, input.position.z, input.position.y },
            { -input.normal.x, input.normal.z, input.normal.y },
            { input.uv.x, 1.0f - input.uv.y }
        };
    }

    inline static glm::vec3 ConvertToVulkanCoordinates(const glm::vec3 &input)
    {
        return { input.x, input.z, input.y };
    }

    void CreateCommandBuffers();
    void CreateCommandPool();
    void CreateFrameBuffers();
    void CreatePipelines();
    void CreateSynchronizationObjects();
    void DestroyCommandBuffers();
    void DestroyFrameBuffers();
    void DestroyPipelines();
    void DestroySynchronizationObjects();

    void BeginFrame(uint32_t &imageIndex);
    void EndFrame(uint32_t &imageIndex);
    void Submit(uint32_t &imageIndex);

    void RecreateSwapChain();

    bool enableDebugging;

    Screen *screen;
    std::unique_ptr<VulkanContext> context;
    std::unique_ptr<VulkanRenderPass> renderPass;

    std::unordered_map<std::thread::id, VkCommandPool> commandPools;
    std::unique_ptr<VulkanCommandManager> commandManager;

    std::unique_ptr<VulkanPipeline> staticPipeline;
    std::unique_ptr<VulkanPipeline> cubeMapPipeline;

    std::unordered_set<uint32_t> bufferIds;
    std::unique_ptr<VulkanBufferManager> bufferManager;

    // Based on number of swap chain images (which is usually 3)
    std::vector<VkFramebuffer> frameBuffers;
    std::vector<VkFence> imagesInFlight;

    // Based on the maximum allowed frames in flight
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    // Active frame in use by the GPU
    uint32_t currentInFlightFrame;
};

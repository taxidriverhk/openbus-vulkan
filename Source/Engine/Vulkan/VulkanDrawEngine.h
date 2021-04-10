#pragma once

#include <memory>
#include <vector>
#include <unordered_set>

#include "Engine/DrawEngine.h"
#include "Engine/Screen.h"

#include "VulkanBufferManager.h"
#include "VulkanContext.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"

class VulkanPipeline;

class VulkanDrawEngine : public DrawEngine
{
public:
    VulkanDrawEngine(Screen *screen, bool enableDebugging);
    ~VulkanDrawEngine();

    void Destroy() override;
    void DrawFrame() override;
    void Initialize() override;
    void LoadIntoBuffer(Entity &entity) override;
    void UpdateCamera(Camera *camera) override;

private:
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

    void ClearBuffers();
    void CreateBuffer();
    void CreatePipeline();

    bool enableDebugging;

    Screen *screen;
    std::unique_ptr<VulkanContext> context;

    std::unique_ptr<VulkanBufferManager> bufferManager;
    std::unique_ptr<VulkanPipeline> pipeline;

    std::unique_ptr<VulkanShader> vertexShader;
    std::unique_ptr<VulkanShader> fragmentShader;

    std::unordered_set<uint32_t> bufferIds;
};

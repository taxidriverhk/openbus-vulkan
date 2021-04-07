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

struct Mesh;
class VulkanPipeline;

class VulkanDrawEngine : public DrawEngine
{
public:
    VulkanDrawEngine(Screen *screen, bool enableDebugging);
    ~VulkanDrawEngine();

    void Destroy() override;
    void DrawFrame() override;
    void Initialize() override;
    void LoadIntoBuffer(Mesh &mesh) override;
    void UpdateCamera(Camera *camera) override;

private:
    // The game treats Z-axis as the up axis, while Vulkan/OpenGL treats Y-axis as the up axis
    // Therefore, a conversion is required
    inline static glm::vec3 ConvertToVulkanCoordinates(glm::vec3 input)
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

    // TODO: replace with entity ID to buffer ID map to allow
    // drawing multiple instances of the same object with different transformations
    std::unordered_set<uint32_t> staticBufferIds;
};

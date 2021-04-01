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
    void LoadIntoBuffer(std::vector<Mesh> &meshes) override;

private:
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

    std::unordered_set<uint32_t> vertexBufferIds;
};

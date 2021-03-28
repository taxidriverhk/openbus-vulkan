#pragma once

#include <memory>
#include <vector>

#include "Engine/DrawEngine.h"
#include "Engine/Screen.h"
#include "VulkanContext.h";

class VulkanDrawEngine : public DrawEngine
{
public:
    VulkanDrawEngine(Screen *screen, bool enableDebugging);
    ~VulkanDrawEngine();

    void CreateContext() override;
    void DestroyContext() override;

private:
    bool enableDebugging;

    Screen *screen;
    std::unique_ptr<VulkanContext> context;

    std::vector<VulkanPipeline> pipelines;
    std::vector<VulkanShader> vertexShaders;
    std::vector<VulkanShader> fragmentShaders;
};

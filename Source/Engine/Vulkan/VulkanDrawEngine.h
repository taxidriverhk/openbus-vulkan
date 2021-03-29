#pragma once

#include <memory>
#include <vector>

#include "Engine/DrawEngine.h"
#include "Engine/Screen.h"

#include "VulkanContext.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"

class VulkanPipeline;

class VulkanDrawEngine : public DrawEngine
{
public:
    VulkanDrawEngine(Screen *screen, bool enableDebugging);
    ~VulkanDrawEngine();

    void CreateContext() override;
    void DestroyContext() override;
private:
    void CreatePipeline();

    bool enableDebugging;

    Screen *screen;
    std::unique_ptr<VulkanContext> context;

    std::unique_ptr<VulkanPipeline> pipeline;
    std::unique_ptr<VulkanShader> vertexShader;
    std::unique_ptr<VulkanShader> fragmentShader;
};

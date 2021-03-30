#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>

#include "DrawEngine.h"
#include "Screen.h"

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void Cleanup();
    void DrawScene();
    void CreateContext(const std::unique_ptr<Screen> &screen);

private:
    std::unique_ptr<DrawEngine> drawEngine;
};

#pragma once

#include <memory>

#include "Camera.h"
#include "DrawEngine.h"
#include "Screen.h"
#include "Entity.h"

class Renderer
{
public:
    Renderer(Camera *camera);
    ~Renderer();

    void Cleanup();
    void DrawScene();
    void CreateContext(const std::unique_ptr<Screen> &screen);
    void LoadScene();

private:
    Camera *camera;
    MeshLoader meshLoader;
    std::unique_ptr<DrawEngine> drawEngine;
};

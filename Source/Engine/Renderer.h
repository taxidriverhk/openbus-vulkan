#pragma once

#include <memory>

#include "Camera.h"
#include "DrawEngine.h"
#include "Entity.h"
#include "Screen.h"
#include "Terrain.h"

class Renderer
{
public:
    Renderer(Camera *camera);
    ~Renderer();

    void Cleanup();
    void DrawScene();
    void CreateContext(Screen *screen);
    void LoadScene();

private:
    Camera *camera;
    MeshLoader meshLoader;
    TerrainLoader terrainLoader;
    std::unique_ptr<DrawEngine> drawEngine;
};

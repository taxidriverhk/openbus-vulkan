#pragma once

#include <memory>

#include "Mesh.h"
#include "Terrain.h"

class Camera;
class DrawEngine;
class Screen;

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

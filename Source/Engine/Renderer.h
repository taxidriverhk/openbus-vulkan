#pragma once

#include <list>
#include <memory>
#include <unordered_map>

#include "Entity.h"
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
    void LoadBackground();
    void LoadBlock(Terrain &terrain, std::vector<Entity> &entities);
    void UnloadEntities(uint32_t blockId);

private:
    Camera *camera;
    std::unique_ptr<DrawEngine> drawEngine;

    // Need to track the entities loaded, so that they can be unloaded from the buffer
    std::unordered_map<uint32_t, std::list<uint32_t>> blockIdEntityIdsMap;
};

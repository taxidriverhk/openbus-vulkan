#pragma once

#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Entity.h"
#include "Mesh.h"
#include "Text.h"
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
    void Initialize(Screen *screen);
    
    void LoadBackground(const std::string &skyBoxImageFilePath, bool enableFog);

    void AddEntity(const Entity &entity);
    void MoveEntity(uint32_t entityId, EntityTransformation transformation);
    void RemoveEntity(uint32_t entityId);

    void LoadBlock(uint32_t blockId, Terrain &terrain, std::vector<Entity> &entities);
    void UnloadBlock(uint32_t blockId);

    void PutText(const Text &text);
    void RemoveText(uint32_t textId);

private:
    Camera *camera;
    FontManager fontManager;
    std::unique_ptr<DrawEngine> drawEngine;

    // Used to track the entities loaded, so that they can be unloaded from the buffer
    std::unordered_map<uint32_t, std::list<uint32_t>> blockIdEntityIdsMap;
    std::unordered_set<uint32_t> textIds;
    std::unordered_set<uint32_t> gameObjectEntityIds;
};

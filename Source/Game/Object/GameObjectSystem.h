#pragma once

#include <list>
#include <memory>
#include <unordered_map>

#include "BaseGameObject.h"

class GameObjectSystem
{
public:
    GameObjectSystem();
    ~GameObjectSystem();

    BaseGameObject *GetCurrentUserObject() const { return currentUserObject.get(); }
    std::list<GameObjectEntity> GetRenderingEntities() const;

    void DespawnGameObject(uint32_t gameObjectId);
    void SpawnGameObject(uint32_t gameObjectId, std::unique_ptr<BaseGameObject> gameObject);
    void SetCurrentUserObject(uint32_t gameObjectId);
    void UpdateState(float deltaTime, const std::list<GameObjectCommand> &commands);

private:
    std::unique_ptr<BaseGameObject> currentUserObject;
    std::unordered_map<uint32_t, std::unique_ptr<BaseGameObject>> gameObjects;
};

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

    bool HasUserObject() const { return currentUserObject != nullptr; }
    BaseGameObject *GetCurrentUserObject() const { return currentUserObject; }
    std::list<GameObjectEntity> GetRenderingEntities() const;

    void Cleanup();
    void DespawnGameObject(uint32_t gameObjectId);
    void SpawnGameObject(uint32_t gameObjectId, std::shared_ptr<BaseGameObject> gameObject);
    void SetCurrentUserObject(uint32_t gameObjectId);
    void UpdateState(float deltaTime, const std::list<ControlCommand> &commands);

private:
    BaseGameObject *currentUserObject;
    std::unordered_map<uint32_t, std::shared_ptr<BaseGameObject>> gameObjects;
};

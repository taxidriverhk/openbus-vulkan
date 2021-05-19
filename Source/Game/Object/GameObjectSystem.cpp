#include "GameObjectSystem.h"

GameObjectSystem::GameObjectSystem()
    : currentUserObject(nullptr)
{
}

GameObjectSystem::~GameObjectSystem()
{
}

std::list<GameObjectEntity> GameObjectSystem::GetRenderingEntities() const
{
    std::list<GameObjectEntity> entities;
    for (const auto &[gameObjectId, gameObject] : gameObjects)
    {
        const auto &gameObjectEntities = gameObject->GetEntities();
        for (const auto &entity : gameObjectEntities)
        {
            entities.push_back(entity);
        }
    }
    return entities;
}

void GameObjectSystem::DespawnGameObject(uint32_t gameObjectId)
{
    if (gameObjects.count(gameObjectId) == 0)
    {
        return;
    }

    gameObjects[gameObjectId]->Destroy();
    gameObjects.erase(gameObjectId);
}

void GameObjectSystem::SpawnGameObject(uint32_t gameObjectId, std::shared_ptr<BaseGameObject> gameObject)
{
    gameObject->Initialize();
    gameObjects[gameObjectId] = gameObject;
}

void GameObjectSystem::SetCurrentUserObject(uint32_t gameObjectId)
{
}

void GameObjectSystem::UpdateState(float deltaTime, const std::list<GameObjectCommand> &commands)
{
    // Could use multi-threading/parallelism to update state of each game object
    // Except for user object/collision-enabled objects which have dependencies

    // TODO: test code to update state iteratively, more code may be added
    for (const auto &[objectId, gameObject] : gameObjects)
    {
        gameObject->Update(deltaTime, commands);
    }
}

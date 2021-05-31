#include "GameObjectSystem.h"

GameObjectSystem::GameObjectSystem(PhysicsSystem *physics)
    : physics(physics),
      currentUserObject(nullptr)
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

void GameObjectSystem::Cleanup()
{
    // Cleans up any game objects that were not removed
    // Should not rely on this function to despawn game objects while in game
    // This function is only used when the game session ends
    for (const auto &[objectId, gameObject] : gameObjects)
    {
        gameObject->Destroy();
    }
    gameObjects.clear();
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
    if (gameObjects.count(gameObjectId) == 0)
    {
        return;
    }

    currentUserObject = gameObjects[gameObjectId].get();
}

void GameObjectSystem::UpdateState(float deltaTime, const std::list<ControlCommand> &commands)
{
    // Could use multi-threading/parallelism to update state of each game object
    // Except for user object/collision-enabled objects which have dependencies

    // TODO: test code to update state iteratively, more code may be added
    for (const auto &[objectId, gameObject] : gameObjects)
    {
        gameObject->Update(deltaTime, commands);
    }
    physics->StepSimulation(deltaTime);
}

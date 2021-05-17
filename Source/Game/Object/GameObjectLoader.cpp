#include "Common/HandledThread.h"
#include "Engine/Entity.h"
#include "BaseGameObject.h"
#include "GameObjectLoader.h"

GameObjectLoader::GameObjectLoader()
    : shouldTerminate(false),
      readyToBuffer(false)
{
}

GameObjectLoader::~GameObjectLoader()
{
}

void GameObjectLoader::AddGameObjectToLoad(const GameObjectLoadRequest &request)
{
}

std::list<std::unique_ptr<Entity>> GameObjectLoader::PollLoadedEntities()
{
    std::list<std::unique_ptr<Entity>> entities;
    return entities;
}

std::list<std::unique_ptr<BaseGameObject>> GameObjectLoader::PollLoadedGameObjects()
{
    std::list<std::unique_ptr<BaseGameObject>> gameObjects;
    return gameObjects;
}

void GameObjectLoader::StartLoadGameObjectThread()
{
}

void GameObjectLoader::TerminateLoadGameObjectThread()
{
}

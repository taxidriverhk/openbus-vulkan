#pragma once

#include <list>
#include <mutex>
#include <queue>
#include <memory>
#include <thread>

#include "Engine/Mesh.h"

struct Entity;
class HandledThread;
class BaseGameObject;

struct GameObjectLoadRequest
{
    bool isUserObject;
    std::string configFilePath;
    glm::vec3 position;
    glm::vec3 rotation;
};

class GameObjectLoader
{
public:
    GameObjectLoader();
    ~GameObjectLoader();

    bool IsReadyToBuffer() const { return readyToBuffer; }
    bool IsReadyToSpawn() const { return readyToSpawn; }

    void AddGameObjectToLoad(const GameObjectLoadRequest &request);
    std::list<std::unique_ptr<Entity>> PollLoadedEntities();
    std::list<std::unique_ptr<BaseGameObject>> PollLoadedGameObjects();

    void StartLoadGameObjectThread();
    void TerminateLoadGameObjectThread();

private:
    static constexpr uint32_t WAIT_TIME_SECONDS = 1;

    std::atomic<bool> readyToSpawn;
    std::list<std::unique_ptr<BaseGameObject>> loadedGameObjects;

    std::atomic<bool> readyToBuffer;
    std::list<std::unique_ptr<Entity>> loadedEntities;

    std::mutex loadQueueMutex;
    std::queue<GameObjectLoadRequest> loadGameObjectQueue;

    bool shouldTerminate;
    std::unique_ptr<HandledThread> loadEntityThread;

    MeshLoader meshLoader;
};

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

enum class GameObjectType
{
    Vehicle,
    Human
};

struct GameObjectLoadRequest
{
    bool isUserObject;
    GameObjectType type;
    std::string configFilePath;
    glm::vec3 position;
    glm::vec3 rotation;
};

struct GameObjectLoadResult
{
    uint32_t id;
    std::shared_ptr<BaseGameObject> object;

    GameObjectLoadResult()
        : id(0),
          object(nullptr)
    {}

    GameObjectLoadResult(const GameObjectLoadResult &other)
    {
        id = other.id;
        object = other.object;
    }
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
    std::list<GameObjectLoadResult> PollLoadedGameObjects();

    void StartLoadGameObjectThread();
    void TerminateLoadGameObjectThread();

private:
    static constexpr uint32_t WAIT_TIME_SECONDS = 1;

    bool LoadVehicleConfig(
        const GameObjectLoadRequest &loadRequest,
        GameObjectLoadResult &vehicleObject,
        std::list<std::unique_ptr<Entity>> &entities);

    uint32_t gameObjectEntityIdCount;

    std::atomic<bool> readyToSpawn;
    std::list<GameObjectLoadResult> loadedGameObjects;

    std::atomic<bool> readyToBuffer;
    std::list<std::unique_ptr<Entity>> loadedEntities;

    std::mutex loadQueueMutex;
    std::queue<GameObjectLoadRequest> loadGameObjectQueue;

    bool shouldTerminate;
    std::unique_ptr<HandledThread> loadEntityThread;

    MeshLoader meshLoader;
};

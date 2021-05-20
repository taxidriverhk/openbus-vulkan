#include <optional>

#include "Common/FileSystem.h"
#include "Common/HandledThread.h"
#include "Common/Identifier.h"
#include "Common/Logger.h"
#include "Config/ConfigReader.h"
#include "Config/GameObjectConfig.h"
#include "Config/ObjectConfig.h"
#include "Engine/Entity.h"
#include "Engine/Image.h"
#include "Engine/Material.h"
#include "BaseGameObject.h"
#include "GameObjectLoader.h"
#include "VehicleGameObject.h"

GameObjectLoader::GameObjectLoader()
    : shouldTerminate(false),
      readyToBuffer(false),
      gameObjectEntityIdCount(0)
{
}

GameObjectLoader::~GameObjectLoader()
{
    if (loadEntityThread != nullptr)
    {
        loadEntityThread->Join();
    }
}

void GameObjectLoader::AddGameObjectToLoad(const GameObjectLoadRequest &request)
{
    loadQueueMutex.lock();
    loadGameObjectQueue.push(request);
    loadQueueMutex.unlock();
}

std::list<std::unique_ptr<Entity>> GameObjectLoader::PollLoadedEntities()
{
    std::list<std::unique_ptr<Entity>> entities;
    if (readyToBuffer)
    {
        entities.splice(entities.end(), loadedEntities);
        readyToBuffer = false;
    }
    return entities;
}

std::list<GameObjectLoadResult> GameObjectLoader::PollLoadedGameObjects()
{
    std::list<GameObjectLoadResult> gameObjects;
    if (readyToSpawn)
    {
        gameObjects.splice(gameObjects.end(), loadedGameObjects);
        readyToSpawn = false;
    }
    return gameObjects;
}

bool GameObjectLoader::LoadVehicleConfig(
    const GameObjectLoadRequest &loadRequest,
    GameObjectLoadResult &vehicleObject,
    std::list<std::unique_ptr<Entity>> &entities)
{
    VehicleConfig vehicleConfig{};
    if (!ConfigReader::ReadConfig(loadRequest.configFilePath, vehicleConfig))
    {
        return false;
    }

    // TODO: may modify this part of code as the vehicle game object can become more complex
    std::string vehicleConfigDirectory = FileSystem::GetParentDirectory(loadRequest.configFilePath);
    std::string chassisObjectFilePath = FileSystem::GetGameObjectFile(vehicleConfigDirectory, vehicleConfig.chassisObject);
    StaticObjectConfig chassisObjectConfig{};
    if (!ConfigReader::ReadConfig(chassisObjectFilePath, chassisObjectConfig))
    {
        return false;
    }

    std::string chassisMeshFilePath = FileSystem::GetModelFile(vehicleConfigDirectory, chassisObjectConfig.mesh);
    std::string chassisTextureFilePath = FileSystem::GetTextureFile(vehicleConfigDirectory, chassisObjectConfig.diffuseMaterial);
    
    uint32_t meshId = Identifier::GenerateIdentifier(chassisMeshFilePath);
    uint32_t materialId = Identifier::GenerateIdentifier(chassisTextureFilePath);
    
    Mesh chassisMesh{};
    chassisMesh.id = meshId;
    if (!meshLoader.LoadFromFile(chassisMeshFilePath, chassisMesh))
    {
        return false;
    }

    Material chassisMaterial{};
    chassisMaterial.id = materialId;
    std::shared_ptr<Image> diffuseImage = std::make_shared<Image>();
    if (!diffuseImage->Load(chassisTextureFilePath, ImageColor::ColorWithAlpha))
    {
        return false;
    }
    chassisMaterial.diffuseImage = diffuseImage;
    chassisMesh.material = std::make_shared<Material>(chassisMaterial);

    const glm::vec3 &translation = loadRequest.position;
    const glm::vec3 &rotation = loadRequest.rotation;
    std::unique_ptr<Entity> chassisEntity = std::make_unique<Entity>();
    chassisEntity->id = Identifier::GenerateIdentifier(IdentifierType::GameObjectEntity, ++gameObjectEntityIdCount);
    chassisEntity->translation = translation;
    chassisEntity->rotation = rotation;
    chassisEntity->scale = { 1.0f, 1.0f, 1.0f };
    chassisEntity->mesh = std::make_shared<Mesh>(chassisMesh);

    GameObjectTransform originTransform{};
    originTransform.worldPosition = translation;
    originTransform.rotation = rotation;
    std::shared_ptr<BaseGameObject> vehicleGameObject = std::make_unique<VehicleGameObject>(chassisEntity->id, originTransform);

    vehicleObject.id = chassisEntity->id;
    vehicleObject.object = vehicleGameObject;
    entities.push_back(std::move(chassisEntity));

    return true;
}

void GameObjectLoader::StartLoadGameObjectThread()
{
    loadEntityThread = std::make_unique<HandledThread>([&]()
        {
            while (!shouldTerminate)
            {
                // Don't push any resource to the list if the rendering/game thread is reading
                if (readyToSpawn || readyToBuffer)
                {
                    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME_SECONDS));
                    continue;
                }

                std::optional<GameObjectLoadRequest> loadRequest = {};
                loadQueueMutex.lock();
                if (!loadGameObjectQueue.empty())
                {
                    loadRequest = loadGameObjectQueue.front();
                    loadGameObjectQueue.pop();
                }
                loadQueueMutex.unlock();

                if (!loadRequest.has_value())
                {
                    continue;
                }

                const GameObjectLoadRequest &loadRequestValue = loadRequest.value();

                GameObjectLoadResult gameObjectLoadResult{};
                gameObjectLoadResult.isUserObject = loadRequestValue.isUserObject;

                std::list<std::unique_ptr<Entity>> entities;
                if (loadRequestValue.type == GameObjectType::Vehicle)
                {
                    Logger::Log(LogLevel::Info, "Loading vehicle game object from {}", loadRequestValue.configFilePath);
                    if (!LoadVehicleConfig(loadRequestValue, gameObjectLoadResult, entities))
                    {
                        Logger::Log(LogLevel::Warning, "Failed to load the vehicle object from {}", loadRequestValue.configFilePath);
                        continue;
                    }
                }
                else if (loadRequestValue.type == GameObjectType::Human)
                {

                }

                loadedGameObjects.push_back(gameObjectLoadResult);
                for (auto &entity : entities)
                {
                    loadedEntities.push_back(std::move(entity));
                }

                readyToBuffer = true;
                readyToSpawn = true;
            }
        },
        [&]()
        {
            TerminateLoadGameObjectThread();
        });
}

void GameObjectLoader::TerminateLoadGameObjectThread()
{
    shouldTerminate = true;
}

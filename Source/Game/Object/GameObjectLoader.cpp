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
#include "Game/Physics/PhysicsSystem.h"
#include "BaseGameObject.h"
#include "GameObjectLoader.h"
#include "VehicleGameObject.h"

GameObjectLoader::GameObjectLoader(PhysicsSystem *physics)
    : physics(physics),
      shouldTerminate(false),
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
    if (!ConfigReader::ReadConfig(loadRequest.configFilePath, vehicleConfig)
        || vehicleConfig.wheels.size() == 0)
    {
        return false;
    }

    // TODO: may modify this part of code as the vehicle game object can become more complex
    std::string vehicleConfigDirectory = FileSystem::GetParentDirectory(loadRequest.configFilePath);

    // Load the chassis object
    std::string chassisObjectFilePath = FileSystem::GetGameObjectFile(vehicleConfigDirectory, vehicleConfig.chassisObject);
    StaticObjectConfig chassisObjectConfig{};
    if (!ConfigReader::ReadConfig(chassisObjectFilePath, chassisObjectConfig))
    {
        return false;
    }

    // Load the wheel objects (assume all wheels use the same model for now)
    std::string &wheelObject = vehicleConfig.wheels[0].object.object;
    std::string wheelObjectFilePath = FileSystem::GetGameObjectFile(vehicleConfigDirectory, wheelObject);
    StaticObjectConfig wheelObjectConfig{};
    if (!ConfigReader::ReadConfig(wheelObjectFilePath, wheelObjectConfig))
    {
        return false;
    }

    // Load the chassis and wheel meshes for rendering
    std::string chassisMeshFilePath = FileSystem::GetModelFile(vehicleConfigDirectory, chassisObjectConfig.mesh);
    std::string chassisTextureFilePath = FileSystem::GetTextureFile(vehicleConfigDirectory, chassisObjectConfig.diffuseMaterial);
    std::string wheelMeshFilePath = FileSystem::GetModelFile(vehicleConfigDirectory, wheelObjectConfig.mesh);
    std::string wheelTextureFilePath = FileSystem::GetTextureFile(vehicleConfigDirectory, wheelObjectConfig.diffuseMaterial);

    uint32_t meshId = Identifier::GenerateIdentifier(chassisMeshFilePath);
    uint32_t materialId = Identifier::GenerateIdentifier(chassisTextureFilePath);
    uint32_t wheelMeshId = Identifier::GenerateIdentifier(wheelMeshFilePath);
    uint32_t wheelMaterialId = Identifier::GenerateIdentifier(wheelTextureFilePath);
    
    Mesh chassisMesh{};
    chassisMesh.id = meshId;
    Mesh wheelMesh{};
    wheelMesh.id = wheelMeshId;

    if (!meshLoader.LoadFromFile(chassisMeshFilePath, chassisMesh)
        || !meshLoader.LoadFromFile(wheelMeshFilePath, wheelMesh))
    {
        return false;
    }

    // Load the textures for the meshes
    Material chassisMaterial{};
    chassisMaterial.id = materialId;
    std::shared_ptr<Image> diffuseImage = std::make_shared<Image>();
    Material wheelMaterial{};
    wheelMaterial.id = wheelMaterialId;
    std::shared_ptr<Image> wheelImage = std::make_shared<Image>();
    if (!diffuseImage->Load(chassisTextureFilePath, ImageColor::ColorWithAlpha)
        || !wheelImage->Load(wheelTextureFilePath, ImageColor::ColorWithAlpha))
    {
        return false;
    }
    chassisMaterial.diffuseImage = diffuseImage;
    chassisMesh.material = std::make_shared<Material>(chassisMaterial);
    wheelMaterial.diffuseImage = wheelImage;
    wheelMesh.material = std::make_shared<Material>(wheelMaterial);

    // Initialize the transformations for both the chassis and the wheels
    const glm::vec3 &translation = loadRequest.position;
    const glm::vec3 &rotation = loadRequest.rotation;
    glm::vec3 centerOfMass =
    {
        vehicleConfig.centerOfMass.x,
        vehicleConfig.centerOfMass.y,
        vehicleConfig.centerOfMass.z
    };

    uint32_t chassisEntityId = Identifier::GenerateIdentifier(IdentifierType::GameObjectEntity, ++gameObjectEntityIdCount);
    std::unique_ptr<Entity> chassisEntity = std::make_unique<Entity>();
    chassisEntity->id = chassisEntityId;
    chassisEntity->translation = translation;
    chassisEntity->rotation = rotation;
    chassisEntity->scale = { 1.0f, 1.0f, 1.0f };
    chassisEntity->mesh = std::make_shared<Mesh>(chassisMesh);
    entities.push_back(std::move(chassisEntity));

    EntityTransformation originTransform{};
    originTransform.translation = translation;
    originTransform.rotation = rotation;

    VehicleGameObjectConstructionInfo createInfo{};
    createInfo.chassisEntityId = chassisEntityId;
    createInfo.chassisStartTransform = originTransform;
    createInfo.boundingBoxSize =
    {
        vehicleConfig.boundingBoxDimensions.x,
        vehicleConfig.boundingBoxDimensions.y,
        vehicleConfig.boundingBoxDimensions.z
    };
    createInfo.centerOfMass =
    {
        vehicleConfig.centerOfMass.x,
        vehicleConfig.centerOfMass.y,
        vehicleConfig.centerOfMass.z
    };
    createInfo.mass = vehicleConfig.mass;
    createInfo.maxSpeed = vehicleConfig.maxSpeed;
    createInfo.engineForce = vehicleConfig.engineForce;
    createInfo.brakeForce = vehicleConfig.brakeForce;
    createInfo.steeringForce = vehicleConfig.steeringForce;
    createInfo.steeringAngle = vehicleConfig.steeringAngle;

    size_t wheelCount = vehicleConfig.wheels.size();
    for (uint32_t i = 0; i < wheelCount; i++)
    {
        WheelConfig &wheelConfig = vehicleConfig.wheels[i];
        EntityConfig &wheelEntityConfig = wheelConfig.object;

        glm::vec3 wheelWorldPosition =
        {
            translation.x + wheelEntityConfig.position.x,
            translation.y + wheelEntityConfig.position.y,
            translation.z + wheelEntityConfig.position.z,
        };
        glm::vec3 wheelTranslation =
        {
            wheelEntityConfig.position.x,
            wheelEntityConfig.position.y,
            wheelEntityConfig.position.z,
        };
        glm::vec3 wheelRotation =
        {
            wheelEntityConfig.rotation.x,
            wheelEntityConfig.rotation.y,
            wheelEntityConfig.rotation.z
        };

        uint32_t wheelEntityId = Identifier::GenerateIdentifier(IdentifierType::GameObjectEntity, ++gameObjectEntityIdCount);
        std::unique_ptr<Entity> wheelEntity = std::make_unique<Entity>();
        wheelEntity->id = wheelEntityId;
        wheelEntity->translation = wheelWorldPosition;
        wheelEntity->rotation = wheelRotation;
        wheelEntity->scale = { 1.0f, 1.0f, 1.0f };
        wheelEntity->mesh = std::make_shared<Mesh>(wheelMesh);
        entities.push_back(std::move(wheelEntity));

        VehicleGameObjectConstructionInfo::WheelInfo wheelInfo{};
        wheelInfo.entityId = wheelEntityId;
        wheelInfo.transform.translation = wheelTranslation;
        wheelInfo.transform.rotation = wheelRotation;
        
        wheelInfo.isTurnable = wheelConfig.isTurnable;
        wheelInfo.hasTorque = wheelConfig.hasTorque;
        wheelInfo.radius = wheelConfig.radius;

        wheelInfo.axle = { wheelConfig.axle.x, wheelConfig.axle.y, wheelConfig.axle.z };
        wheelInfo.direction = { wheelConfig.direction.x, wheelConfig.direction.y, wheelConfig.direction.z };

        wheelInfo.suspensionRestLength = wheelConfig.suspensionRestLength;
        wheelInfo.suspensionStiffness = wheelConfig.suspensionStiffness;
        wheelInfo.wheelsDampingRelaxation = wheelConfig.wheelsDampingRelaxation;
        wheelInfo.wheelsDampingCompression = wheelConfig.wheelsDampingCompression;
        wheelInfo.frictionSlip = wheelConfig.frictionSlip;
        wheelInfo.rollInfluence = wheelConfig.rollInfluence;

        createInfo.wheels.push_back(wheelInfo);
    }
    
    std::shared_ptr<BaseGameObject> vehicleGameObject = std::make_unique<VehicleGameObject>(createInfo, physics);

    vehicleObject.id = chassisEntityId;
    vehicleObject.object = vehicleGameObject;
    vehicleObject.isUserObject = loadRequest.isUserObject;

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

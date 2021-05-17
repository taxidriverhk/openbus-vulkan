#include "Common/FileSystem.h"
#include "Common/HandledThread.h"
#include "Common/Util.h"
#include "Common/Logger.h"
#include "Common/Timer.h"
#include "Config/ConfigReader.h"
#include "Engine/Camera.h"
#include "Engine/Screen.h"
#include "Engine/Renderer.h"
#include "Game/Object/GameObjectLoader.h"
#include "Game/Object/GameObjectSystem.h"
#include "Game/Object/VehicleGameObject.h"
#include "Control.h"
#include "Game.h"
#include "Map.h"

Game::Game()
    : gameStarted(false),
      shouldEndGame(false),
      readyToRender(false),
      gameSettings{}
{
}

Game::~Game()
{
}

void Game::Cleanup()
{
    Logger::Log(LogLevel::Info, "Stopping game loop thread");
    gameLoopThread->Join();
    Logger::Log(LogLevel::Info, "Stopping other threads");
    mapLoader->TerminateLoadBlocksThread();
    gameObjectLoader->TerminateLoadGameObjectThread();
    Logger::Log(LogLevel::Info, "Cleaning up graphics context");
    renderer->Cleanup();
    Logger::Log(LogLevel::Info, "Closing screen");
    screen->Close();
}

void Game::InitializeComponents()
{
    int screenWidth = gameSettings.graphicsSettings.screenWidth,
        screenHeight = gameSettings.graphicsSettings.screenHeight,
        maxDistance = gameSettings.graphicsSettings.maxViewableDistance;

    controlManager = std::make_unique<ControlManager>();

    gameObjectSystem = std::make_unique<GameObjectSystem>();
    gameObjectLoader = std::make_unique<GameObjectLoader>();

    camera = std::make_unique<Camera>(screenWidth, screenHeight);
    camera->SetZFar(static_cast<float>(maxDistance));

    std::string screenTitle = Util::FormatWindowTitle("Game Screen");
    screen = std::make_unique<Screen>(screenWidth, screenHeight, screenTitle);
    screen->Create();

    renderer = std::make_unique<Renderer>(camera.get());
    renderer->Initialize(screen.get());
}

void Game::InitializeSettings(const GameSessionConfig &startConfig)
{
    std::string settingsFilePath = FileSystem::GetSettingsFile();
    if (!ConfigReader::ReadConfig(settingsFilePath, gameSettings))
    {
        throw std::runtime_error("Failed to read config file from settings.json");
    }
}

void Game::InitializeState(const GameSessionConfig &startConfig)
{
    // Load the map and its block loader
    MapLoadSettings &mapLoadSettings = gameSettings.mapLoadSettings;
    map = std::make_unique<Map>(startConfig.mapConfigPath);
    mapLoader = std::make_unique<MapLoader>(map.get(), mapLoadSettings);
}

void Game::AddUserGameObject(const GameObjectLoadRequest &request)
{
    if (!gameStarted)
    {
        return;
    }


}

void Game::SetShouldEndGame(const bool &shouldEndGame)
{
    this->shouldEndGame = shouldEndGame;
}

bool Game::ShouldQuit()
{
    return shouldEndGame || screen->ShouldClose();
}

void Game::Start(const GameSessionConfig &startConfig)
{
    gameStartConfig = startConfig;
    gameStarted = true;
    shouldEndGame = false;

    Logger::Log(LogLevel::Info, "Initializing game settings");
    InitializeSettings(startConfig);

    Logger::Log(LogLevel::Info, "Initializing game state");
    InitializeState(startConfig);

    Logger::Log(LogLevel::Info, "Initializing graphics context");
    InitializeComponents();

    Logger::Log(LogLevel::Info, "Creating a separate thread for game logic loop");
    gameLoopThread = std::make_unique<HandledThread>(
        [&]()
        {
            RunGameLoop();
        },
        [&]()
        {
            SetShouldEndGame(true);
        });

    Logger::Log(LogLevel::Info, "Creating a separate thread for loading map resources");
    mapLoader->StartLoadBlocksThread();

    Logger::Log(LogLevel::Info, "Creating a separate thread for loading moving entities");
    gameObjectLoader->StartLoadGameObjectThread();

    RunMainLoop();
}

void Game::RenderScene()
{
    for (const auto &renderingEntity : gameObjectSystem->GetRenderingEntities())
    {
        EntityTransformation entityTransformation{};
        entityTransformation.translation = renderingEntity.worldPosition;
        entityTransformation.rotation = renderingEntity.rotation;
        entityTransformation.scale = { 1.0f, 1.0f, 1.0f };
        renderer->MoveEntity(renderingEntity.entityId, entityTransformation);
    }

    renderer->DrawScene();
    readyToRender = false;
}

// TODO: includes only for the hard-coded vehicle game object
#include "Engine/Image.h"
#include "Engine/Material.h"

void Game::RunMainLoop()
{
    Logger::Log(LogLevel::Info, "Loading uniform background");
    renderer->LoadBackground(map->GetSkyBoxImageFilePath(), gameSettings.graphicsSettings.enableFog);
    screen->Show();
    
    // TODO: test code for game object which is moving
    Entity vehicleEntity{};
    vehicleEntity.id = 999;
    vehicleEntity.scale = { 1.0, 1.0, 1.0 };
    vehicleEntity.translation = { 50, 50, 10 };
    MeshLoader loader;
    Mesh vehicleMesh{};
    loader.LoadFromFile("objects\\Test Objects\\models\\car.obj", vehicleMesh);
    vehicleMesh.id = 999;
    vehicleMesh.material = std::make_shared<Material>();
    vehicleMesh.material->id = 999;
    vehicleMesh.material->diffuseImage = std::make_shared<Image>("objects\\Test Objects\\models\\car-yellow.bmp");
    vehicleEntity.mesh = std::make_shared<Mesh>(vehicleMesh);
    renderer->AddEntity(vehicleEntity);

    Logger::Log(LogLevel::Info, "Entering the rendering loop");
    while (!ShouldQuit())
    {
        // Read any input events from the SFML screen and then queue them
        // so that the game thread can process them
        controlManager->QueueEvents(screen->PollEvent());

        // Update the block position based on the camera position
        // so that it knows if blocks should be added/removed
        bool blockPositionChanged = map->UpdateBlockPosition(camera->GetPosition());
        if (blockPositionChanged)
        {
            std::unordered_set<MapBlockPosition> mapBlockPositionsToKeep = mapLoader->GetAdjacentBlocks();
            std::list<uint32_t> mapBlockIdsToUnload = map->UnloadBlocks(mapBlockPositionsToKeep);
            Logger::Log(LogLevel::Info, "Position updated, unloading {} blocks", mapBlockIdsToUnload.size());
            for (uint32_t blockId : mapBlockIdsToUnload)
            {
                renderer->UnloadBlock(blockId);
            }

            // TODO: also check to see if there are requests to remove game objects from the graphics context

        }
        // Load the resources into buffer if found
        if (mapLoader->IsReadyToBuffer())
        {
            MapBlockResources mapBlockResource = mapLoader->PollLoadedResources();
            renderer->LoadBlock(
                mapBlockResource.blockId,
                mapBlockResource.terrain,
                mapBlockResource.entities);
        }
        else
        {
            mapLoader->AddBlocksToLoad();
        }

        // TODO: Load/remove the game object meshes into/from graphics context
        // works in the same way as the map block loading thread
        if (gameObjectLoader->IsReadyToBuffer())
        {

        }

        // Redraw the scene only if the game thread has completed its updates
        if (readyToRender)
        {
            RenderScene();
        }
    }

    // TODO: test code for removing the hard-coded moving entity
    renderer->RemoveEntity(vehicleEntity.id);

    Cleanup();
    gameStarted = false;
}

void Game::RunGameLoop()
{
    // This is not the drawing framerate
    // but the framerate for updating physics, game state, etc.
    int targetFrameRate = gameSettings.graphicsSettings.targetFrameRate;
    float timePerFrame = 1 / static_cast<float>(targetFrameRate);

    // TODO: test code to ensure that the game state logic is working
    // spawn game object only after the mesh is loaded
    std::list<GameObjectCommand> commands;
    gameObjectSystem->SpawnGameObject(1, std::make_unique<VehicleGameObject>(999));

    float timeSinceLastUpdate = 0.0f;
    Timer timer;
    while (!ShouldQuit())
    {
        // TODO: If there are game objects ready to spawn, then poll from the loader
        // thread and then put into the game object system for state/physics updates in future frames
        if (gameObjectLoader->IsReadyToSpawn())
        {

        }
        // TOOD: Otherwise, determine if game objects should spawn or despawn
        // Then put the game object into loader thread for loading the meshes and initializing the phyiscs
        else
        {

        }

        timeSinceLastUpdate += timer.DeltaTime();
        // This is to accommodate a fixed time step for updating the physics accurately
        // without affecting the framerate for drawing
        while (timeSinceLastUpdate > timePerFrame)
        {
            timeSinceLastUpdate -= timePerFrame;

            HandleInputCommands(timePerFrame);
            UpdateState(timePerFrame, commands);
        }

        readyToRender = true;
    }
}

// TODO: could be moved into a separate class for handling?
void Game::HandleInputCommands(float deltaTime)
{
    float movementSpeed = 100;
    for (const ControlCommand &command : controlManager->PollCommands())
    {
        switch (command.operation)
        {
        case ControlCommandOperation::CameraMoveForward:
            camera->MoveBy(0.0f, movementSpeed * deltaTime, 0.0f);
            break;
        case ControlCommandOperation::CameraMoveBackward:
            camera->MoveBy(0.0f, -movementSpeed * deltaTime, 0.0f);
            break;
        case ControlCommandOperation::CameraMoveLeft:
            camera->MoveBy(-movementSpeed * deltaTime, 0.0f, 0.0f);
            break;
        case ControlCommandOperation::CameraMoveRight:
            camera->MoveBy(movementSpeed * deltaTime, 0.0f, 0.0f);
            break;
        case ControlCommandOperation::CameraMoveUp:
            camera->MoveBy(0.0f, 0.0f, movementSpeed * deltaTime);
            break;
        case ControlCommandOperation::CameraMoveDown:
            camera->MoveBy(0.0f, 0.0f, -movementSpeed * deltaTime);
            break;
        case ControlCommandOperation::CameraRotateCounterClockwise:
            camera->RotateBy(0.0f, -movementSpeed * deltaTime, 0.0f);
            break;
        case ControlCommandOperation::CameraRotateClockwise:
            camera->RotateBy(0.0f, movementSpeed * deltaTime, 0.0f);
            break;
        case ControlCommandOperation::ToggleFrameRateDisplay:
            gameStartConfig.enableFrameRateDisplay = !gameStartConfig.enableFrameRateDisplay;
            break;
        }
    }
}

void Game::UpdateState(float deltaTime, const std::list<GameObjectCommand> &commands)
{
    // TODO: populate the commands
    gameObjectSystem->UpdateState(deltaTime, commands);
}

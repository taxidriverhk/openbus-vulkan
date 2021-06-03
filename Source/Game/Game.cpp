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
#include "Map/Map.h"
#include "Map/MapLoader.h"
#include "Control.h"
#include "Game.h"
#include "View.h"

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
    Logger::Log(LogLevel::Info, "Cleaning up game objects");
    gameObjectSystem->Cleanup();
    Logger::Log(LogLevel::Info, "Cleaning up graphics context");
    renderer->Cleanup();
    Logger::Log(LogLevel::Info, "Closing screen");
    screen->Close();
}

void Game::InitializeComponents()
{
    int screenWidth = gameSettings.graphicsSettings.screenWidth,
        screenHeight = gameSettings.graphicsSettings.screenHeight;

    controlManager = std::make_unique<ControlManager>();

    physicsSystem = std::make_unique<PhysicsSystem>();
    gameObjectSystem = std::make_unique<GameObjectSystem>(physicsSystem.get());
    gameObjectLoader = std::make_unique<GameObjectLoader>(physicsSystem.get());

    camera = std::make_unique<Camera>(screenWidth, screenHeight);
    view = std::make_unique<View>(camera.get(), gameObjectSystem.get(), gameSettings);

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

    GameObjectLoadRequest requestWithCurrentPosition(request);
    requestWithCurrentPosition.position = view->GetWorldPosition();
    gameObjectLoader->AddGameObjectToLoad(requestWithCurrentPosition);
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
        renderer->MoveEntity(renderingEntity.entityId, renderingEntity.transform);
    }

    renderer->DrawScene();
    readyToRender = false;
}

void Game::RunMainLoop()
{
    Logger::Log(LogLevel::Info, "Loading uniform background");
    renderer->LoadBackground(map->GetSkyBoxImageFilePath(), gameSettings.graphicsSettings.enableFog);
    screen->Show();

    Logger::Log(LogLevel::Info, "Entering the rendering loop");
    while (!ShouldQuit())
    {
        // Read any input events from the SFML screen and then queue them
        // so that the game thread can process them
        controlManager->QueueEvents(screen->PollEvent());

        // Update the block position based on the camera position
        // so that it knows if blocks should be added/removed
        bool blockPositionChanged = map->UpdateBlockPosition(view->GetWorldPosition());
        if (blockPositionChanged)
        {
            std::unordered_set<MapBlockPosition> mapBlockPositionsToKeep = mapLoader->GetAdjacentBlocks();
            std::list<uint32_t> mapBlockIdsToUnload = map->UnloadBlocks(mapBlockPositionsToKeep);
            Logger::Log(LogLevel::Info, "Position updated, unloading {} blocks", mapBlockIdsToUnload.size());
            for (uint32_t blockId : mapBlockIdsToUnload)
            {
                renderer->UnloadBlock(blockId);
                // TODO: could be not thread-safe, should let game thread make this call
                physicsSystem->RemoveSurface(blockId);
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
            std::list<std::unique_ptr<Entity>> gameObjectEntities = gameObjectLoader->PollLoadedEntities();
            for (const auto &gameObjectEntity : gameObjectEntities)
            {
                renderer->AddEntity(*gameObjectEntity);
            }
        }

        // Redraw the scene only if the game thread has completed its updates
        if (readyToRender)
        {
            view->UpdateView();
            RenderScene();
        }
    }

    Cleanup();
    gameStarted = false;
}

void Game::RunGameLoop()
{
    // This is not the drawing framerate
    // but the framerate for updating physics, game state, etc.
    int targetFrameRate = gameSettings.graphicsSettings.targetFrameRate;
    float timePerFrame = 1 / static_cast<float>(targetFrameRate);

    float timeSinceLastUpdate = 0.0f;
    Timer timer;
    while (!ShouldQuit())
    {
        // TODO: If there are game objects ready to spawn, then poll from the loader
        // thread and then put into the game object system for state/physics updates in future frames
        if (gameObjectLoader->IsReadyToSpawn())
        {
            std::list<GameObjectLoadResult> gameObjects = gameObjectLoader->PollLoadedGameObjects();
            for (auto &gameObject : gameObjects)
            {
                gameObjectSystem->SpawnGameObject(gameObject.id, gameObject.object);
                if (gameObject.isUserObject)
                {
                    gameObjectSystem->SetCurrentUserObject(gameObject.id);
                }
            }
        }
        // TOOD: Otherwise, determine if game objects should spawn or despawn
        // Then put the game object into loader thread for loading the meshes and initializing the phyiscs
        else
        {

        }

        // Load collision meshes into the physics world
        if (mapLoader->IsReadyToAdd())
        {
            MapBlockSurfaces mapBlockSurface = mapLoader->PollLoadedSurfaces();
            physicsSystem->AddSurface(mapBlockSurface.blockId, mapBlockSurface.collisionMeshes);
        }

        timeSinceLastUpdate += timer.DeltaTime();
        // This is to accommodate a fixed time step for updating the physics accurately
        // without affecting the framerate for drawing
        while (timeSinceLastUpdate > timePerFrame)
        {
            timeSinceLastUpdate -= timePerFrame;

            HandleInputCommands(timePerFrame);
            UpdateState(timePerFrame);
        }

        readyToRender = true;
    }
}

// TODO: could be moved into a separate class for handling?
void Game::HandleInputCommands(float deltaTime)
{
    for (const ControlCommand &command : controlManager->PollCommands())
    {
        ControlCommandOperation operation = command.operation;
        if (operation > ControlCommandOperation::CameraControlStart
            && operation < ControlCommandOperation::CameraControlEnd)
        {
            view->Move(command, deltaTime);
        }
        else if (operation == ControlCommandOperation::SwitchView)
        {
            view->SwitchView();
        }
        else if (operation == ControlCommandOperation::ToggleFrameRateDisplay)
        {
            gameStartConfig.enableFrameRateDisplay = !gameStartConfig.enableFrameRateDisplay;
        }
        else if (operation > ControlCommandOperation::UserObjectControlStart
            && operation < ControlCommandOperation::UserObjectControlEnd)
        {
            if (view->GetViewMode() != ViewMode::Free)
            {
                gameObjectCommands.push_back(command);
            }
        }
    }
}

void Game::UpdateState(float deltaTime)
{
    gameObjectSystem->UpdateState(deltaTime, gameObjectCommands);
    gameObjectCommands.clear();
}

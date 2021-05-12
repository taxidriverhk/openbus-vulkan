#include "Common/FileSystem.h"
#include "Common/HandledThread.h"
#include "Common/Util.h"
#include "Common/Logger.h"
#include "Common/Timer.h"
#include "Config/ConfigReader.h"
#include "Engine/Camera.h"
#include "Engine/Screen.h"
#include "Engine/Renderer.h"
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
    gameSettings.mapLoadSettings.maxAdjacentBlocks = 1;
    gameSettings.graphicsSettings.enableFog = true;
    gameSettings.graphicsSettings.targetFrameRate = 120;
    gameSettings.graphicsSettings.maxViewableDistance = 1200;
    gameSettings.graphicsSettings.screenWidth = 1920;
    gameSettings.graphicsSettings.screenHeight = 1080;
}

void Game::InitializeState(const GameSessionConfig &startConfig)
{
    // Load the map and its block loader
    MapLoadSettings &mapLoadSettings = gameSettings.mapLoadSettings;
    map = std::make_unique<Map>(startConfig.mapConfigPath);
    mapLoader = std::make_unique<MapLoader>(map.get(), mapLoadSettings);
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

    Logger::Log(LogLevel::Info, "Creating a separate thread for loading resources");
    mapLoader->StartLoadBlocksThread();

    RunMainLoop();
}

void Game::RenderScene()
{
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
        bool blockPositionChanged = map->UpdateBlockPosition(camera->GetPosition());
        if (blockPositionChanged)
        {
            std::unordered_set<MapBlockPosition> mapBlockPositionsToKeep = mapLoader->GetAdjacentBlocks();
            std::list<uint32_t> mapBlockIdsToUnload = map->UnloadBlocks(mapBlockPositionsToKeep);
            Logger::Log(LogLevel::Info, "Position updated, unloading {} blocks", mapBlockIdsToUnload.size());
            for (uint32_t blockId : mapBlockIdsToUnload)
            {
                renderer->UnloadEntities(blockId);
            }
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

        // Redraw the scene only if the game thread has completed its updates
        if (readyToRender)
        {
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

void Game::UpdateState(float deltaTime)
{
}

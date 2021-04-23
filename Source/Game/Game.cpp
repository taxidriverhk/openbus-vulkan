#include "Common/Util.h"
#include "Common/Logger.h"
#include "Common/Timer.h"
#include "Engine/Camera.h"
#include "Engine/Screen.h"
#include "Engine/Renderer.h"
#include "Control.h"
#include "Game.h"

Game::Game()
    : gameStarted(false),
      shouldEndGame(false),
      readyToRender(false)
{
    std::string screenTitle = Util::FormatWindowTitle("Game Screen");

    controlManager = std::make_unique<ControlManager>();
    camera = std::make_unique<Camera>(SCREEN_WIDTH, SCREEN_HEIGHT);
    screen = std::make_unique<Screen>(SCREEN_WIDTH, SCREEN_HEIGHT, screenTitle);
    renderer = std::make_unique<Renderer>(camera.get());

    number = 0;
}

Game::~Game()
{
}

void Game::Cleanup()
{
    renderer->Cleanup();
    screen->Close();
}

void Game::InitializeComponents()
{
    screen->Create();
    renderer->CreateContext(screen.get());
}

void Game::InitializeState()
{
}

void Game::SetShouldEndGame(const bool &shouldEndGame)
{
    this->shouldEndGame = shouldEndGame;
}

bool Game::ShouldQuit()
{
    return shouldEndGame || screen->ShouldClose();
}

void Game::Start()
{
    gameStarted = true;
    shouldEndGame = false;
    RunMainLoop();
}

void Game::RenderScene()
{
    renderer->DrawScene();
    readyToRender = false;
}

void Game::RunMainLoop()
{
    Logger::Log(LogLevel::Info, "Initializing graphics context");
    InitializeComponents();

    Logger::Log(LogLevel::Info, "Initializing game state");
    InitializeState();

    // TODO: this code should be moved to a separate thread
    // (ideally a separate class like MapBlockManager)
    Logger::Log(LogLevel::Info, "Loading map");
    renderer->LoadScene();

    screen->Show();

    Logger::Log(LogLevel::Info, "Creating a separate thread for game logic loop");
    std::thread gameLoopThread(&Game::RunGameLoop, this);

    Logger::Log(LogLevel::Info, "Entering the rendering loop");
    while (!ShouldQuit())
    {
        controlManager->QueueEvents(screen->PollEvent());

        if (readyToRender)
        {
            RenderScene();
        }
    }

    gameLoopThread.join();
    Cleanup();
    gameStarted = false;
}

void Game::RunGameLoop()
{
    int targetFrameRate = 120;
    float timePerFrame = 1 / static_cast<float>(targetFrameRate);

    float timeSinceLastUpdate = 0.0f;
    Timer timer;
    while (!ShouldQuit())
    {
        timeSinceLastUpdate += timer.DeltaTime();
        // This is to accommodate a fixed time step for updating the physics
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

void Game::HandleInputCommands(float deltaTime)
{
    float movementSpeed = 20;
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
            number++;
            break;
        }
    }
}

void Game::UpdateState(float deltaTime)
{
}

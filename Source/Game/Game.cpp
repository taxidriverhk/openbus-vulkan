#include "Game.h"
#include "Common/Logger.h"

Game::Game()
{
    std::string screenTitle = Util::FormatWindowTitle("Game Screen");

    gameStarted = false;
    shouldEndGame = false;
    camera = std::make_unique<Camera>(SCREEN_WIDTH, SCREEN_HEIGHT);
    screen = std::make_unique<Screen>(SCREEN_WIDTH, SCREEN_HEIGHT, screenTitle);
    renderer = std::make_unique<Renderer>(camera.get());
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
    StartGameLoop();
}

void Game::StartGameLoop()
{
    Logger::Log(LogLevel::Info, "Initializing graphics context");
    InitializeComponents();

    Logger::Log(LogLevel::Info, "Initializing game state");
    InitializeState();

    // TODO: this code should be moved to a separate thread
    // (ideally a separate class like MapBlockManager)
    Logger::Log(LogLevel::Info, "Loading map");
    renderer->LoadScene();
    camera->MoveTo(50, 50, 10);
    std::chrono::steady_clock::time_point startTime = std::chrono::high_resolution_clock::now();

    Logger::Log(LogLevel::Info, "Entering the game loop");
    screen->Show();
    while (!ShouldQuit())
    {
        float deltaTime = Util::DeltaTime();

        // TODO: test code to verify that the camera works
        std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        camera->Rotate(0, 20 * time, 0);

        screen->Refresh();
        renderer->DrawScene();
    }
    Cleanup();
    gameStarted = false;
}

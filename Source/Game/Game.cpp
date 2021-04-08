#include "Game.h"

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
    renderer->CreateContext(screen);
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
    InitializeComponents();
    InitializeState();

    // TODO: this code should be moved to a separate thread
    // (ideally a separate class like MapBlockManager)
    renderer->LoadScene();

    std::chrono::steady_clock::time_point startTime = std::chrono::high_resolution_clock::now();

    while (!ShouldQuit())
    {
        float deltaTime = Util::DeltaTime();

        // TODO: test code to show that the camera works
        std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        float cameraX = 5 * glm::cos(time);
        float cameraY = 5 * glm::sin(time);
        camera->MoveTo(cameraX, cameraY, 5.0f);

        screen->Refresh();
        renderer->DrawScene();
    }
    Cleanup();
    gameStarted = false;
}

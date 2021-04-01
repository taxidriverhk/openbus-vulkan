#include "Game.h"

Game::Game()
{
    std::string screenTitle = Util::FormatWindowTitle("Game Screen");

    gameStarted = false;
    shouldEndGame = false;
    screen = std::make_unique<Screen>(SCREEN_WIDTH, SCREEN_HEIGHT, screenTitle);
    renderer = std::make_unique<Renderer>();
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

    // TODO: this code should be moved to somewhere else
    renderer->LoadScene();

    while (!ShouldQuit())
    {
        screen->Refresh();
        renderer->DrawScene();
    }
    Cleanup();
    gameStarted = false;
}

#include "Game.h"

Game::Game()
{
    std::string screenTitle = Util::FormatWindowTitle("Game Screen");

    shouldEndGame = false;
    screen = std::make_unique<Screen>(SCREEN_WIDTH, SCREEN_HEIGHT, screenTitle);
    renderer = std::make_unique<Renderer>();
}

Game::~Game()
{
}

void Game::Cleanup()
{
    screen->Close();
}

void Game::InitializeComponents()
{
    screen->Open();
    renderer->PrepareContext();
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
    SetShouldEndGame(false);
    StartGameLoop();
}

void Game::StartGameLoop()
{
    InitializeComponents();
    while (!ShouldQuit())
    {
        screen->Refresh();
    }
    Cleanup();
}

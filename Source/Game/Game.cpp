#include "Game.h"

Game::Game()
    : gameEnded(false),
      screen(std::make_unique<Screen>(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenBus Vulkan|" APP_VERSION "|Game Screen"))
{
}

Game::~Game()
{
}

void Game::SendEndSignal()
{
    gameEnded = true;
}

bool Game::ShouldQuit()
{
    return gameEnded || screen->ShouldClose();
}

void Game::Start()
{
    gameEnded = false;
    StartGameLoop();
}

void Game::StartGameLoop()
{
    screen->OpenScreen();
    while (!ShouldQuit())
    {
        screen->Refresh();
    }
    screen->Close();
}

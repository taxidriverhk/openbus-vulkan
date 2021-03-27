#pragma once

#include "Common/Util.h"
#include "Engine/Screen.h"
#include "Engine/Renderer.h"

#include <memory>

class Game
{

public:
    Game();
    ~Game();

    void SetShouldEndGame(const bool &shouldEndGame);
    void Start();

private:
    static constexpr int SCREEN_WIDTH = 1280;
    static constexpr int SCREEN_HEIGHT = 720;

    void Cleanup();
    void InitializeComponents();
    bool ShouldQuit();
    void StartGameLoop();

    bool shouldEndGame;
    std::unique_ptr<Screen> screen;
    std::unique_ptr<Renderer> renderer;
};

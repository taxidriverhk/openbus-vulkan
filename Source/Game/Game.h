#pragma once

#include "Engine/Screen.h"

#include <memory>

class Game
{

public:
    Game();
    ~Game();

    void SendEndSignal();
    void Start();

private:
    static constexpr int SCREEN_WIDTH = 1280;
    static constexpr int SCREEN_HEIGHT = 720;

    bool ShouldQuit();
    void StartGameLoop();

    bool gameEnded;
    std::unique_ptr<Screen> screen;
};

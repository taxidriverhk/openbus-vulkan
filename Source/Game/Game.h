#pragma once

#include <memory>

#include "Common/Util.h"
#include "Engine/Camera.h"
#include "Engine/Screen.h"
#include "Engine/Renderer.h"

class Game
{

public:
    Game();
    ~Game();

    bool GetGameStarted() const { return gameStarted; }

    void SetShouldEndGame(const bool &shouldEndGame);
    void Start();

private:
    static constexpr int SCREEN_WIDTH = 1280;
    static constexpr int SCREEN_HEIGHT = 720;

    void Cleanup();
    void InitializeComponents();
    bool ShouldQuit();
    void StartGameLoop();

    bool gameStarted;
    bool shouldEndGame;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Screen> screen;
    std::unique_ptr<Renderer> renderer;
};

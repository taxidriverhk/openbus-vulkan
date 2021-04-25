#pragma once

#include <memory>
#include <thread>

class ControlManager;
class MapLoader;
class Camera;
class Screen;
class Renderer;

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
    void InitializeState();

    bool ShouldQuit();

    void RunMainLoop();
    void RenderScene();

    void RunGameLoop();
    void HandleInputCommands(float deltaTime);
    void UpdateState(float deltaTime);

    bool gameStarted;
    bool shouldEndGame;

    std::atomic<bool> readyToRender;

    std::unique_ptr<ControlManager> controlManager;
    std::unique_ptr<MapLoader> mapLoader;

    std::unique_ptr<Camera> camera;
    std::unique_ptr<Screen> screen;
    std::unique_ptr<Renderer> renderer;
};

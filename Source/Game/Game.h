#pragma once

#include <memory>
#include <thread>

#include "Config/GameConfig.h"
#include "Config/SettingsConfig.h"

class HandledThread;
class ControlManager;
class Map;
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

    void Cleanup();
    void SetShouldEndGame(const bool &shouldEndGame);
    void Start(const GameSessionConfig &startConfig);

private:
    void InitializeComponents();
    void InitializeSettings(const GameSessionConfig &startConfig);
    void InitializeState(const GameSessionConfig &startConfig);

    bool ShouldQuit();

    void RunMainLoop();
    void RenderScene();

    void RunGameLoop();
    void HandleInputCommands(float deltaTime);
    void UpdateState(float deltaTime);

    GameSettings gameSettings;
    GameSessionConfig gameStartConfig;

    bool gameStarted;
    bool shouldEndGame;
    std::unique_ptr<HandledThread> gameLoopThread;

    std::atomic<bool> readyToRender;

    std::unique_ptr<ControlManager> controlManager;
    std::unique_ptr<Map> map;
    std::unique_ptr<MapLoader> mapLoader;

    std::unique_ptr<Camera> camera;
    std::unique_ptr<Screen> screen;
    std::unique_ptr<Renderer> renderer;
};

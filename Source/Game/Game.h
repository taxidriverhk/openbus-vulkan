#pragma once

#include <list>
#include <memory>
#include <thread>

#include "Common/Identifier.h"
#include "Config/GameConfig.h"
#include "Config/SettingsConfig.h"
#include "Engine/Text.h"
#include "Object/BaseGameObject.h"

struct GameObjectLoadRequest;

class HandledThread;
class ControlManager;
class GameObjectSystem;
class GameObjectLoader;
class PhysicsSystem;
class Map;
class MapLoader;
class Camera;
class Screen;
class Renderer;
class View;

class Game
{
public:
    Game();
    ~Game();

    bool GetGameStarted() const { return gameStarted; }

    void AddUserGameObject(const GameObjectLoadRequest &request);
    void Cleanup();
    void SetShouldEndGame(const bool &shouldEndGame);
    void Start(const GameSessionConfig &startConfig);

private:
    static constexpr uint32_t DEBUG_INFO_ENTITY_ID = static_cast<uint32_t>(IdentifierType::DebugInfo);

    void InitializeComponents();
    void InitializeSettings(const GameSessionConfig &startConfig);
    void InitializeState(const GameSessionConfig &startConfig);

    void PrepareDebugInfo();

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

    std::list<ControlCommand> gameObjectCommands;
    std::unique_ptr<PhysicsSystem> physicsSystem;
    std::unique_ptr<GameObjectSystem> gameObjectSystem;
    std::unique_ptr<GameObjectLoader> gameObjectLoader;

    std::unique_ptr<ControlManager> controlManager;
    std::unique_ptr<Map> map;
    std::unique_ptr<MapLoader> mapLoader;

    std::unique_ptr<Camera> camera;
    std::unique_ptr<View> view;

    std::unique_ptr<Screen> screen;
    std::unique_ptr<Renderer> renderer;
};

#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Config/SettingsConfig.h"
#include "Control.h"

class Camera;
class GameObjectSystem;

enum class ViewMode : int
{
    Free = 0,
    FirstPerson = 1,
    ThirdPerson = 2
};

class View
{
public:
    View(Camera *camera, GameObjectSystem *gameObjectSystem, const GameSettings &gameSettings);
    ~View();

    glm::vec3 GetWorldPosition() const { return worldPosition; }
    ViewMode GetViewMode() const { return VIEW_MODES[currentViewModeIndex]; }

    void SetMovementSpeed(float speed) { movementSpeed = speed; }
    void SetViewableDistance(float distance)
    { 
        viewableDistance = distance;
        camera->SetZFar(viewableDistance);
    }

    void UpdateView();
    void SwitchView();

    void Move(const ControlCommand &controlCommand, float deltaTime);

private:
    static constexpr float MAX_ALLOWABLE_ZOOM = 50.0f;
    static constexpr float MIN_ALLOWABLE_ZOOM = 1.0f;
    static constexpr float MAX_ALLOWABLE_ANGLE = 89.0f;
    static constexpr float MIN_ALLOWABLE_ANGLE = -89.0f;

    static constexpr int VIEW_MODE_COUNT = 3;
    static constexpr ViewMode VIEW_MODES[] =
    {
        ViewMode::Free,
        ViewMode::FirstPerson,
        ViewMode::ThirdPerson
    };

    // Applicable to both free and third/first person modes
    float movementSpeed;
    float viewableDistance;
    float angleChangeSensitivity;
    float zoomSensitivity;
    float zoom;

    // Used for third/first person mode
    float followYaw;
    float followPitch;
    glm::vec3 distanceFromObject;

    // Used for free mode
    float pitch;
    float yaw;
    float roll;

    glm::vec3 worldPosition;
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;

    int currentViewModeIndex;

    Camera *camera;
    GameObjectSystem *gameObjectSystem;
};

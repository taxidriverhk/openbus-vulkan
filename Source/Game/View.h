#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Control.h"

class Camera;
class GameObjectSystem;

enum class ViewMode
{
    Free,
    Follow
};

class View
{
public:
    View(Camera *camera, GameObjectSystem *gameObjectSystem);
    ~View();

    glm::vec3 GetWorldPosition() const { return worldPosition; }

    void SetMode(ViewMode nextMode) { mode = nextMode; }
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
    static constexpr float MAX_ALLOWABLE_PITCH = 90.0f;
    static constexpr float MIN_ALLOWABLE_PITCH = 0.0f;

    float movementSpeed;
    float viewableDistance;
    float zoom;

    // Used for follow mode
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

    ViewMode mode;

    Camera *camera;
    GameObjectSystem *gameObjectSystem;
};

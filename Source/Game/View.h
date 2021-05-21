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

    void Move(ControlCommandOperation controlCommand, float deltaTime);

private:
    float movementSpeed;
    float viewableDistance;

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

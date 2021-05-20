#include "Engine/Camera.h"
#include "Object/GameObjectSystem.h"
#include "View.h"

View::View(Camera *camera, GameObjectSystem *gameObjectSystem)
    : camera(camera),
      gameObjectSystem(gameObjectSystem),
      mode(ViewMode::Free),
      worldPosition{},
      viewableDistance(0.0f),
      movementSpeed(10.0f),
      pitch(0.0f),
      yaw(0.0f),
      roll(0.0f)
{
}

View::~View()
{
}

void View::UpdateView()
{
    // If free camera, then simply update camera with free mode position
    if (mode == ViewMode::Free)
    {
        camera->MoveTo(worldPosition.x, worldPosition.y, worldPosition.z);
        camera->RotateTo(pitch, yaw, roll);
    }
    // If follow, then get the current user object's world position, then update the camera
    else if (mode == ViewMode::Follow)
    {
        const GameObjectTransform transform = gameObjectSystem->GetCurrentUserObject()->GetWorldTransform();
        const glm::vec3 &position = transform.worldPosition;
        camera->MoveTo(position.x, position.y, position.z);
    }
}

void View::Move(ControlCommandOperation controlCommand, float deltaTime)
{
    switch (controlCommand)
    {
    case ControlCommandOperation::CameraMoveLeft:
        worldPosition.x -= movementSpeed * deltaTime;
        break;
    case ControlCommandOperation::CameraMoveRight:
        worldPosition.x += movementSpeed * deltaTime;
        break;
    case ControlCommandOperation::CameraMoveForward:
        worldPosition.y += movementSpeed * deltaTime;
        break;
    case ControlCommandOperation::CameraMoveBackward:
        worldPosition.y -= movementSpeed * deltaTime;
        break;
    case ControlCommandOperation::CameraMoveUp:
        worldPosition.z += movementSpeed * deltaTime;
        break;
    case ControlCommandOperation::CameraMoveDown:
        worldPosition.z -= movementSpeed * deltaTime;        
        break;
    case ControlCommandOperation::CameraRotateCounterClockwise:
        yaw -= movementSpeed * deltaTime;
        break;
    case ControlCommandOperation::CameraRotateClockwise:
        yaw += movementSpeed * deltaTime;
        break;
    }
}

void View::SwitchView()
{
    if (mode == ViewMode::Free && gameObjectSystem->HasUserObject())
    {
        mode = ViewMode::Follow;
    }
    else
    {
        mode = ViewMode::Free;
    }
}

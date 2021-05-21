#include "Engine/Camera.h"
#include "Object/GameObjectSystem.h"
#include "View.h"

View::View(Camera *camera, GameObjectSystem *gameObjectSystem)
    : camera(camera),
      gameObjectSystem(gameObjectSystem),
      mode(ViewMode::Free),
      viewableDistance(0.0f),
      movementSpeed(10.0f),
      worldPosition{},
      front(0.0f, 1.0f, 0.0f),
      right(1.0f, 0.0f, 0.0f),
      up(0.0, 0.0, 1.0f),
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
        camera->SetCamera(worldPosition, front, right, up);
    }
    // If follow, then get the current user object's world position, then update the camera
    else if (mode == ViewMode::Follow)
    {
        const GameObjectTransform transform = gameObjectSystem->GetCurrentUserObject()->GetWorldTransform();
        const glm::vec3 &rotation = transform.rotation;
        float carYawRadians = glm::radians<float>(rotation.z);

        constexpr float pitchRadians = glm::radians<float>(10);
        glm::vec3 distanceFromObject = { -15, 15, 5 };

        glm::vec3 position = transform.worldPosition;
        float sinYaw = glm::sin(carYawRadians),
              cosYaw = glm::cos(carYawRadians);
        float sinPitch = glm::sin(pitchRadians),
              cosPitch = glm::cos(pitchRadians);
        position.x += (distanceFromObject.x * sinYaw);
        position.y += (distanceFromObject.y * cosYaw);
        position.z += distanceFromObject.z;

        glm::vec3 objectFront = { sinYaw, -cosYaw, -sinPitch };
        objectFront = glm::normalize(objectFront);

        glm::vec3 objectRight = glm::cross(objectFront, up);
        camera->SetCamera(position, objectFront, objectRight, up);
    }
}

void View::Move(ControlCommandOperation controlCommand, float deltaTime)
{
    glm::vec3 movement{};
    switch (controlCommand)
    {
    case ControlCommandOperation::CameraMoveLeft:
        movement.x = -movementSpeed * deltaTime;
        break;
    case ControlCommandOperation::CameraMoveRight:
        movement.x = movementSpeed * deltaTime;
        break;
    case ControlCommandOperation::CameraMoveForward:
        movement.y = movementSpeed * deltaTime;
        break;
    case ControlCommandOperation::CameraMoveBackward:
        movement.y = -movementSpeed * deltaTime;
        break;
    case ControlCommandOperation::CameraMoveUp:
        movement.z = movementSpeed * deltaTime;
        break;
    case ControlCommandOperation::CameraMoveDown:
        movement.z = -movementSpeed * deltaTime;
        break;
    case ControlCommandOperation::CameraRotateCounterClockwise:
        yaw -= movementSpeed * deltaTime;
        break;
    case ControlCommandOperation::CameraRotateClockwise:
        yaw += movementSpeed * deltaTime;
        break;
    }

    float yawRadians = glm::radians<float>(yaw),
          pitchRadians = glm::radians<float>(pitch);
    float sinYaw = glm::sin(yawRadians),
          cosYaw = glm::cos(yawRadians);
    float cosPitch = glm::cos(pitchRadians),
          sinPitch = glm::sin(pitchRadians);

    front.x = cosPitch * sinYaw;
    front.y = cosPitch * cosYaw;
    front.z = sinPitch;
    front = glm::normalize(front);
    right = glm::cross(front, up);

    worldPosition += right * movement.x;
    worldPosition += front * movement.y;
    worldPosition.z += movement.z;
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

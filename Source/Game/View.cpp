#include "Engine/Camera.h"
#include "Object/GameObjectSystem.h"
#include "View.h"

View::View(Camera *camera, GameObjectSystem *gameObjectSystem, const GameSettings &gameSettings)
    : camera(camera),
      gameObjectSystem(gameObjectSystem),
      currentViewModeIndex(0),
      viewableDistance(static_cast<float>(gameSettings.graphicsSettings.maxViewableDistance)),
      movementSpeed(gameSettings.controlSettings.cameraMovementSpeed),
      angleChangeSensitivity(gameSettings.controlSettings.cameraAngleChangeSensitivity),
      zoomSensitivity(gameSettings.controlSettings.cameraZoomSensitivity),
      worldPosition{},
      front(0.0f, 1.0f, 0.0f),
      right(1.0f, 0.0f, 0.0f),
      up(0.0, 0.0, 1.0f),
      pitch(0.0f),
      yaw(0.0f),
      roll(0.0f),
      zoom(1.0f),
      followYaw(0.0f),
      followPitch(-10.0f),
      distanceFromObject(-15.0f, 15.0f, 5.0f)
{
}

View::~View()
{
}

void View::UpdateView()
{
    // If free mode, then simply update camera with free mode position
    if (VIEW_MODES[currentViewModeIndex] == ViewMode::Free)
    {
        camera->SetCamera(worldPosition, front, right, up);
    }
    // If third person mode, then get the current user object's world position, then update the camera
    else if (VIEW_MODES[currentViewModeIndex] == ViewMode::FirstPerson
        || VIEW_MODES[currentViewModeIndex] == ViewMode::ThirdPerson)
    {
        const GameObjectTransform transform = gameObjectSystem->GetCurrentUserObject()->GetWorldTransform();
        const glm::vec3 &rotation = transform.rotation;

        float objectYawRadians = glm::radians<float>(rotation.z - followYaw);
        float objectPitchRadians = glm::radians<float>(rotation.x - followPitch);
        glm::vec3 position = transform.worldPosition;

        float sinYaw = glm::sin(objectYawRadians),
              cosYaw = glm::cos(objectYawRadians);
        float sinPitch = glm::sin(objectPitchRadians),
              cosPitch = glm::cos(objectPitchRadians);
        position.x -= (distanceFromObject.x * sinYaw);
        position.y -= (distanceFromObject.y * cosYaw);
        position.z += distanceFromObject.z;

        glm::vec3 objectFront = { -sinYaw, cosYaw, -sinPitch };
        objectFront = glm::normalize(objectFront);
        glm::vec3 objectRight = glm::cross(objectFront, up);

        camera->SetCamera(position, objectFront, objectRight, up);
    }

    camera->Zoom(zoom);
}

void View::Move(const ControlCommand &controlCommand, float deltaTime)
{
    // Camera movement that is applicable to both free and follow modes
    switch (controlCommand.operation)
    {
    case ControlCommandOperation::CameraZoomIn:
        zoom += zoomSensitivity;
        break;
    case ControlCommandOperation::CameraZoomOut:
        zoom -= zoomSensitivity;
        break;
    case ControlCommandOperation::CameraAngleChange:
    {
        float intensityX = controlCommand.movement.deltaX * angleChangeSensitivity,
              intensityY = controlCommand.movement.deltaY * angleChangeSensitivity;
        if (VIEW_MODES[currentViewModeIndex] == ViewMode::Free)
        {
            yaw -= intensityX;
            pitch += intensityY;

            pitch = std::clamp(pitch, MIN_ALLOWABLE_ANGLE, MAX_ALLOWABLE_PITCH_ANGLE);
            yaw = std::clamp(yaw, MIN_ALLOWABLE_ANGLE, MAX_ALLOWABLE_YAW_ANGLE);
        }
        else
        {
            followYaw += intensityX;
            followPitch += intensityY;

            followYaw = std::clamp(followYaw, MIN_ALLOWABLE_ANGLE, MAX_ALLOWABLE_YAW_ANGLE);
            followPitch = std::clamp(followPitch, MIN_ALLOWABLE_ANGLE, MAX_ALLOWABLE_PITCH_ANGLE);
        }
    }
        break;
    }

    zoom = std::clamp(zoom, MIN_ALLOWABLE_ZOOM, MAX_ALLOWABLE_ZOOM);

    // Don't move the camera manually if not in free mode
    // The user object will make the camera move by changing its own position
    if (VIEW_MODES[currentViewModeIndex] != ViewMode::Free)
    {
        return;
    }

    glm::vec3 movement{};
    switch (controlCommand.operation)
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
    if (currentViewModeIndex >= 0
        && currentViewModeIndex < VIEW_MODE_COUNT - 1
        && gameObjectSystem->HasUserObject())
    {
        currentViewModeIndex++;
    }
    else
    {
        currentViewModeIndex = 0;
    }
}

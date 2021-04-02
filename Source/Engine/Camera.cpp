#include "Camera.h"

Camera::Camera(int screenWidth, int screenHeight)
    : zNear(0.1),
      zFar(100.0f),
      aspect((float) screenWidth / screenHeight),
      zoomFactor(1.0),
      fieldOfView(zoomFactor * glm::radians(45.0f)),
      position(0.0, 0.0, 0.0),
      target(0.0, 0.0, 0.0f),
      up(0.0, 0.0, 1.0)
{
}

Camera::~Camera()
{
}

void Camera::MoveTo(float x, float y, float z)
{
    position.x = x;
    position.y = y;
    position.z = z;
}

void Camera::Rotate(float pitch, float yaw, float roll)
{
}

void Camera::Zoom(float factor)
{
}

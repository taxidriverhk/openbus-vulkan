#include <assert.h>

#include "Camera.h"

Camera::Camera(int screenWidth, int screenHeight)
    : zNear(0.1f),
      zFar(1000.0f),
      zoomFactor(1.0f),
      position(0.0, 0.0, 0.0),
      target(0.0, 0.0, 0.0f),
      up(0.0, 0.0, 1.0f)
{
    assert("Screen width and height must be greater than zero", screenWidth > 0 && screenHeight > 0);

    aspect = (float)screenWidth / screenHeight;
    fieldOfView = zoomFactor * glm::radians(45.0f);
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

#include <assert.h>

#include "Camera.h"

Camera::Camera(int screenWidth, int screenHeight)
    : zNear(0.1f),
      zFar(1000.0f),
      zoomFactor(1.0f),
      position(0.0f, 0.0f, 0.0f),
      front(0.0f, 1.0f, 0.0f),
      right(1.0f, 0.0f, 0.0f),
      up(0.0, 0.0, 1.0f)
{
    assert(("Screen width and height must be greater than zero", screenWidth > 0 && screenHeight > 0));

    aspect = (float)screenWidth / screenHeight;
    fieldOfView = zoomFactor * glm::radians(45.0f);
}

Camera::~Camera()
{
}

void Camera::MoveTo(float x, float y, float z)
{
    position += right * x;
    position += front * y;
    position.z += z;
}

void Camera::Rotate(float pitch, float yaw, float roll)
{
    float pitchRadians = glm::radians<float>(pitch);
    float yawRadians = glm::radians<float>(yaw);

    float cosYaw = glm::cos(yawRadians);
    float sinYaw = glm::sin(yawRadians);
    float cosPitch = glm::cos(pitchRadians);
    float sinPitch = glm::sin(pitchRadians);

    front.x = cosPitch * sinYaw;
    front.y = cosPitch * cosYaw;
    front.z = sinPitch;

    right.x = cosYaw;
    right.y = -sinYaw;

    up = glm::normalize(glm::cross(right, front));
}

void Camera::Zoom(float factor)
{
}

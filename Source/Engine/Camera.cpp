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

void Camera::SetCamera(glm::vec3 position, glm::vec3 front, glm::vec3 right, glm::vec3 up)
{
    this->position = position;
    this->front = front;
    this->right = right;
    this->up = up;
}

void Camera::Zoom(float factor)
{
    if (factor <= 0.0f)
    {
        factor = 0.001f;
    }
    zoomFactor = factor;
}

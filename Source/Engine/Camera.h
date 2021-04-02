#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
    Camera(int screenWidth, int screenHeight);
    ~Camera();

    glm::vec3 GetPosition() const { return position; }
    glm::vec3 GetTarget() const { return target; }
    glm::vec3 GetUp() const { return up; }

    float GetZNear() const { return zNear; }
    float GetZFar() const { return zFar; }

    float GetAspect() const { return aspect; }
    float GetFieldOfView() const { return fieldOfView; }

    void MoveTo(float x, float y, float z);
    void Rotate(float pitch, float yaw, float roll);
    void Zoom(float factor);

private:
    float zNear;
    float zFar;
    float aspect;
    float fieldOfView;
    float zoomFactor;

    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
};

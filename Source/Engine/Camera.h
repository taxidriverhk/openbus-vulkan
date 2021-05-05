#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class Camera
{
public:
    Camera(int screenWidth, int screenHeight);
    ~Camera();

    glm::vec3 GetPosition() const { return position; }
    glm::vec3 GetFront() const { return front; }
    glm::vec3 GetUp() const { return up; }

    float GetZNear() const { return zNear; }
    float GetZFar() const { return zFar; }

    float GetAspect() const { return aspect; }
    float GetFieldOfView() const { return fieldOfView; }

    void SetZFar(float zFarToSet) { zFar = zFarToSet; }

    void MoveTo(float x, float y, float z);
    void MoveBy(float x, float y, float z);
    void RotateTo(float pitch, float yaw, float roll);
    void RotateBy(float pitch, float yaw, float roll);
    void Zoom(float factor);

private:
    static constexpr float MAX_ROTATION_ANGLE_DEGREES = 360;

    void ClampAngles(float &pitch, float &yaw, float &roll);

    float zNear;
    float zFar;
    float aspect;
    float fieldOfView;
    float zoomFactor;

    glm::vec3 angles;
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;
};

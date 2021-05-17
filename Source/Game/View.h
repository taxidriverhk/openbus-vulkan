#pragma once

#include <glm/glm.hpp>

class Camera;

enum class ViewMode
{
    Free,
    Follow
};

class View
{
public:
    View(Camera *camera);
    ~View();

    void UpdateView();
    void SwitchView(ViewMode mode);

    void SetFreeModePosition(const glm::vec3 &position);

private:
    glm::vec3 freeModePosition;
    ViewMode mode;
    Camera *camera;
};

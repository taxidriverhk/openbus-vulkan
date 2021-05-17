#include "Engine/Camera.h"
#include "View.h"

View::View(Camera *camera)
    : camera(camera),
      mode(ViewMode::Free),
      freeModePosition{}
{
}

View::~View()
{
}

void View::UpdateView()
{
    // If free camera, then simply update camera with free mode position
    // If follow, then get the current user object's world position, then update the camera
}

void View::SetFreeModePosition(const glm::vec3 &position)
{
    freeModePosition = position;
}

void View::SwitchView(ViewMode mode)
{
}

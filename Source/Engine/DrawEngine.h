#pragma once

class Camera;
struct Entity;

class DrawEngine
{
public:
    virtual ~DrawEngine() {}

    virtual void ClearBuffers() = 0;
    virtual void Destroy() = 0;
    virtual void DrawFrame() = 0;
    virtual void Initialize() = 0;
    virtual void LoadIntoBuffer(Entity &entity) = 0;
    virtual void UpdateCamera(Camera *camera) = 0;
};

#pragma once

class Camera;
struct Mesh;

class DrawEngine
{
public:
    virtual ~DrawEngine() {}

    virtual void ClearBuffers() = 0;
    virtual void Destroy() = 0;
    virtual void DrawFrame() = 0;
    virtual void Initialize() = 0;
    virtual void LoadIntoBuffer(Mesh &mesh) = 0;
    virtual void UpdateCamera(Camera *camera) = 0;
};

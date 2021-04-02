#pragma once

#include <vector>

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
    virtual void LoadIntoBuffer(uint32_t bufferId, std::vector<Mesh> &meshes) = 0;
    virtual void UpdateCamera(Camera *camera) = 0;
};

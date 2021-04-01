#pragma once

#include <vector>

struct Mesh;

class DrawEngine
{
public:
    virtual ~DrawEngine() {}

    virtual void ClearBuffers() = 0;
    virtual void Destroy() = 0;
    virtual void DrawFrame() = 0;
    virtual void Initialize() = 0;
    virtual void LoadIntoBuffer(std::vector<Mesh> &meshes) = 0;
};

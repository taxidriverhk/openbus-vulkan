#pragma once

class DrawEngine
{
public:
    virtual ~DrawEngine() {}

    virtual void Destroy() = 0;
    virtual void DrawFrame() = 0;
    virtual void Initialize() = 0;
};

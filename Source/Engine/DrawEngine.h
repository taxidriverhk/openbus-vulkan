#pragma once

class DrawEngine
{
public:
    virtual ~DrawEngine() {}

    virtual void CreateContext() = 0;
    virtual void DestroyContext() = 0;
};

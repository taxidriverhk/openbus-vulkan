#pragma once

#include <memory>

#include <btBulletDynamicsCommon.h>

class PhysicsSystem
{
public:
    PhysicsSystem();
    ~PhysicsSystem();

private:
    std::unique_ptr<btDynamicsWorld> world;
};

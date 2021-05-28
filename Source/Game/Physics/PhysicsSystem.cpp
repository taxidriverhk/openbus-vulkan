#include "PhysicsSystem.h"

PhysicsSystem::PhysicsSystem()
{
    // TODO: test code to make sure bullet physics is installed properly
    world = std::make_unique<btDiscreteDynamicsWorld>(nullptr, nullptr, nullptr, nullptr);
}

PhysicsSystem::~PhysicsSystem()
{
}

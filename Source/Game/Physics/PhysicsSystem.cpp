#include <btBulletDynamicsCommon.h>

#include "Game/Object/VehicleGameObject.h"
#include "PhysicsSystem.h"

PhysicsSystem::PhysicsSystem()
{
    // TODO: test code to make sure bullet physics is installed properly
    collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
    dispatcher = std::make_unique<btCollisionDispatcher>(collisionConfig.get());
    broadPhaseInterface = std::make_unique<btDbvtBroadphase>();
    solver = std::make_unique<btSequentialImpulseConstraintSolver>();
    world = std::make_unique<btDiscreteDynamicsWorld>(
        dispatcher.get(),
        broadPhaseInterface.get(),
        solver.get(),
        collisionConfig.get());
}

PhysicsSystem::~PhysicsSystem()
{
}

void PhysicsSystem::AddSurface()
{
}

void PhysicsSystem::StepSimulation(float deltaTime)
{
}

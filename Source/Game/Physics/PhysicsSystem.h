#pragma once

#include <memory>

class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btCollisionConfiguration;
class btDynamicsWorld;

class VehicleGameObject;

class PhysicsSystem
{
public:
    PhysicsSystem();
    ~PhysicsSystem();

    btDynamicsWorld *GetDynamicsWorld() const { return world.get(); }

    void AddSurface();
    void StepSimulation(float deltaTime);

private:
    std::unique_ptr<btCollisionDispatcher> dispatcher;
    std::unique_ptr<btBroadphaseInterface> broadPhaseInterface;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
    std::unique_ptr<btCollisionConfiguration> collisionConfig;

    std::unique_ptr<btDynamicsWorld> world;
};

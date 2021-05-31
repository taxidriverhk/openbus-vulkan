#pragma once

#include <memory>
#include <unordered_map>

class btBoxShape;
class btBroadphaseInterface;
class btCollisionDispatcher;
class btCollisionConfiguration;
class btDynamicsWorld;
class btMotionState;
class btRigidBody;
class btSequentialImpulseConstraintSolver;

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
    static constexpr float GRAVITY = -9.81f;

    std::unique_ptr<btCollisionDispatcher> dispatcher;
    std::unique_ptr<btBroadphaseInterface> broadPhaseInterface;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
    std::unique_ptr<btCollisionConfiguration> collisionConfig;

    std::unique_ptr<btDynamicsWorld> world;

    // TODO: test shape
    std::unique_ptr<btBoxShape> groundShape;
    std::unique_ptr<btMotionState> groundMotionState;

    // Entity ID to collision surfaces (ex. terrain, road, etc.)
    std::unordered_map<uint32_t, std::unique_ptr<btRigidBody>> groundSurfaces;
};

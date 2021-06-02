#pragma once

#include <memory>
#include <unordered_map>

#include "Collision.h"

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

    void AddSurface(uint32_t blockId, const std::vector<CollisionMesh> &collisionMeshes);
    void RemoveSurface(uint32_t blockId);

    void StepSimulation(float deltaTime);

private:
    static constexpr float GRAVITY = -9.81f;

    std::unique_ptr<btCollisionDispatcher> dispatcher;
    std::unique_ptr<btBroadphaseInterface> broadPhaseInterface;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
    std::unique_ptr<btCollisionConfiguration> collisionConfig;

    std::unique_ptr<btDynamicsWorld> world;

    // Map block ID to collision surfaces (ex. terrain, road, etc.)
    std::unordered_map<uint32_t, std::vector<std::unique_ptr<CollisionBody>>> groundCollisionSurfaces;
};

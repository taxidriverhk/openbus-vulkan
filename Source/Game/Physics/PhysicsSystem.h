#pragma once

#include <memory>
#include <unordered_map>

#include "Engine/Vertex.h"
#include "Collision.h"
#include "DebugDrawer.h"

class btBoxShape;
class btBroadphaseInterface;
class btCollisionDispatcher;
class btCollisionConfiguration;
class btIDebugDraw;
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
    btIDebugDraw *GetDebugDrawer() const { return debugDrawer.get(); }

    void AddSurface(uint32_t blockId, const std::vector<CollisionMesh> &collisionMeshes);
    void RemoveSurface(uint32_t blockId);

    bool IsDebugDrawingEnabled() const { return enableDebugDrawing; }
    void EnableDebugDrawing(bool enabled) { enableDebugDrawing = enabled; }
    DebugSegments GetDebugDrawing() const;
    void ResetDebugDrawing();

    void StepSimulation(float deltaTime);

private:
    static constexpr float GRAVITY = -9.81f;

    DebugDrawer *GetChildDebugDrawer() const { return dynamic_cast<DebugDrawer *>(debugDrawer.get()); }
    void RedrawDebuggingWorld();

    std::unique_ptr<btCollisionDispatcher> dispatcher;
    std::unique_ptr<btBroadphaseInterface> broadPhaseInterface;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
    std::unique_ptr<btCollisionConfiguration> collisionConfig;

    std::unique_ptr<btDynamicsWorld> world;

    bool enableDebugDrawing;
    std::unique_ptr<btIDebugDraw> debugDrawer;

    // Map block ID to collision surfaces (ex. terrain, road, etc.)
    std::unordered_map<uint32_t, std::vector<std::unique_ptr<CollisionBody>>> groundCollisionSurfaces;
};

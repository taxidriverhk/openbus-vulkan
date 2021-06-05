#include <execution>
#include <mutex>
#include <btBulletDynamicsCommon.h>

#include "Common/Logger.h"
#include "Game/Object/VehicleGameObject.h"
#include "PhysicsSystem.h"

static constexpr int VERTEX_STRIDE = 3 * sizeof(btScalar);
static constexpr int INDEX_STRIDE = 3 * sizeof(uint32_t);

PhysicsSystem::PhysicsSystem()
{
    collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
    dispatcher = std::make_unique<btCollisionDispatcher>(collisionConfig.get());
    broadPhaseInterface = std::make_unique<btDbvtBroadphase>();
    solver = std::make_unique<btSequentialImpulseConstraintSolver>();
    world = std::make_unique<btDiscreteDynamicsWorld>(
        dispatcher.get(),
        broadPhaseInterface.get(),
        solver.get(),
        collisionConfig.get());

    world->setGravity(btVector3(0.0f, 0.0f, GRAVITY));

    enableDebugDrawing = false;
    debugDrawer = std::make_unique<DebugDrawer>();
    world->setDebugDrawer(debugDrawer.get());
}

PhysicsSystem::~PhysicsSystem()
{
    // Must manually remove each collision object from the world
    // Otherwise, the game could crash due to the way Bullet Physics cleans up the world
    for (const auto &[blockId, collisionMeshes] : groundCollisionSurfaces)
    {
        for (const auto &collisionMesh : collisionMeshes)
        {
            world->removeCollisionObject(collisionMesh->body.get());
        }
    }
    groundCollisionSurfaces.clear();
}

void PhysicsSystem::AddSurface(uint32_t blockId, const std::vector<CollisionMesh> &collisionMeshes)
{
    // Adds surfaces to the physics system that are static
    Logger::Log(LogLevel::Info, "Loading {} collision surfaces into physics for map block ID {}",
        collisionMeshes.size(), blockId);
    
    std::vector<bool> results;
    std::mutex updateMutex;
    std::for_each(
        std::execution::par_unseq,
        collisionMeshes.begin(),
        collisionMeshes.end(),
        [&](const auto &collisionMesh)
        {
            std::unique_ptr<CollisionBody> collisionBody = std::make_unique<CollisionBody>();
            std::unique_ptr<btTriangleMesh> triangleMesh = std::make_unique<btTriangleMesh>();

            const std::vector<glm::vec3> &vertices = collisionMesh.vertices;
            const std::vector<uint32_t> &indices = collisionMesh.indices;
            for (size_t i = 0; i < indices.size(); i += 3)
            {
                const glm::vec3 &p1 = vertices[indices[i]];
                const glm::vec3 &p2 = vertices[indices[i + 1]];
                const glm::vec3 &p3 = vertices[indices[i + 2]];

                triangleMesh->addTriangle(
                    btVector3(p1.x, p1.y, p1.z),
                    btVector3(p2.x, p2.y, p2.z),
                    btVector3(p3.x, p3.y, p3.z));
            }

            collisionBody->mesh = std::move(triangleMesh);
            std::unique_ptr<btCollisionShape> collisionShape = std::make_unique<btBvhTriangleMeshShape>(
                collisionBody->mesh.get(), true);
            collisionBody->shape = std::move(collisionShape);

            std::unique_ptr<btMotionState> collisionMotionState = std::make_unique<btDefaultMotionState>();
            collisionBody->motionState = std::move(collisionMotionState);

            // Mass of zero means the body will never move regardless of the momentum
            btRigidBody::btRigidBodyConstructionInfo collisionShapeInfo(
                0, collisionBody->motionState.get(), collisionBody->shape.get());
            std::unique_ptr<btRigidBody> collisionSurface = std::make_unique<btRigidBody>(collisionShapeInfo);
            collisionBody->body = std::move(collisionSurface);

            updateMutex.lock();
            groundCollisionSurfaces[blockId].push_back(std::move(collisionBody));
            world->addRigidBody(groundCollisionSurfaces[blockId].back()->body.get());
            updateMutex.unlock();

            return true;
        }
    );

    RedrawDebuggingWorld();
}

DebugSegments PhysicsSystem::GetDebugDrawing() const
{
    return GetChildDebugDrawer()->GetDrawingSegments();
};

void PhysicsSystem::RedrawDebuggingWorld()
{
    if (!enableDebugDrawing)
    {
        return;
    }

    GetChildDebugDrawer()->SetIsDrawingStatic(true);
    GetChildDebugDrawer()->Clear();
    world->debugDrawWorld();
    GetChildDebugDrawer()->SetIsDrawingStatic(false);
}

void PhysicsSystem::RemoveSurface(uint32_t blockId)
{
    if (groundCollisionSurfaces.count(blockId) == 0)
    {
        return;
    }

    Logger::Log(LogLevel::Info, "Unloading collision surfaces from physics for map block ID {}", blockId);
    const auto &collisionMeshes = groundCollisionSurfaces[blockId];
    for (const auto &collisionMesh : collisionMeshes)
    {
        world->removeCollisionObject(collisionMesh->body.get());
    }

    RedrawDebuggingWorld();

    groundCollisionSurfaces.erase(blockId);
}

void PhysicsSystem::ResetDebugDrawing()
{
    if (enableDebugDrawing)
    {
        GetChildDebugDrawer()->Clear();
    }
}

void PhysicsSystem::StepSimulation(float deltaTime)
{
    world->stepSimulation(deltaTime);
}

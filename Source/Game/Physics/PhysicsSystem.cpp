#include <btBulletDynamicsCommon.h>

#include "Game/Object/VehicleGameObject.h"
#include "PhysicsSystem.h"

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

    // TODO: test code to make sure the basic setup is working on a flat ground
    groundShape = std::make_unique<btBoxShape>(btVector3(5000.0f, 5000.f, 3.0f));
    groundMotionState = std::make_unique<btDefaultMotionState>(
        btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    btRigidBody::btRigidBodyConstructionInfo groundInfo(0, groundMotionState.get(), groundShape.get());

    std::unique_ptr<btRigidBody> groundSurface = std::make_unique<btRigidBody>(groundInfo);
    groundSurface->setFriction(1.0f);
    groundSurface->setRestitution(0.9f);
    groundSurface->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);

    groundSurfaces[999] = std::move(groundSurface);
    world->addRigidBody(groundSurfaces[999].get());
}

PhysicsSystem::~PhysicsSystem()
{
    // Must manually remove each collision object from the world
    // Otherwise, the game could crash due to the way Bullet Physics cleans up the world
    int collisionObjectsSize = world->getNumCollisionObjects();
    btCollisionObjectArray &collisionObjects = world->getCollisionObjectArray();
    for (int i = 0; i < collisionObjectsSize; i++)
    {
        world->removeCollisionObject(collisionObjects[i]);
    }
    groundSurfaces.clear();
}

void PhysicsSystem::AddSurface()
{
}

void PhysicsSystem::StepSimulation(float deltaTime)
{
    world->stepSimulation(deltaTime);
}

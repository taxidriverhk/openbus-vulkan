#include <btBulletDynamicsCommon.h>

#include "Game/Physics/PhysicsSystem.h"
#include "VehicleGameObject.h"

VehicleGameObject::VehicleGameObject(uint32_t bodyEntityId, const GameObjectTransform &originTransform, PhysicsSystem *physics)
    : baseTransform(originTransform),
      origin(originTransform.worldPosition),
      bodyEntityId(bodyEntityId),
      physics(physics),
      angle(0.0f),
      speed(0)
{
}

VehicleGameObject::~VehicleGameObject()
{
}

void VehicleGameObject::Destroy()
{
}

void VehicleGameObject::Initialize()
{
    btRigidBody::btRigidBodyConstructionInfo chassisInfo(100.0, nullptr, nullptr);
    btRaycastVehicle::btVehicleTuning tuning;
    btDynamicsWorld *world = physics->GetDynamicsWorld();

    chassis = std::make_unique<btRigidBody>(chassisInfo);
    raycaster = std::make_unique<btDefaultVehicleRaycaster>(world);
    vehicle = std::make_unique<btRaycastVehicle>(tuning, chassis.get(), raycaster.get());

    world->addVehicle(vehicle.get());
}

void VehicleGameObject::Update(float deltaTime, const std::list<ControlCommand> &commands)
{
    // Do not call stepSimulation as the game object system will make the call
    // This function should simply need to apply whatever input (ex. throttle) the user gives
    // TODO: just some test code to move object
    // will be removed once this is integrated with bullet physics
    for (const ControlCommand &command : commands)
    {
        switch (command.operation)
        {
        case ControlCommandOperation::VehicleAccelerate:
            speed += 0.1f;
            break;
        case ControlCommandOperation::VehicleBrake:
            speed -= 0.1f;
            break;
        case ControlCommandOperation::VehicleSteerLeft:
            angle += 0.1f;
            break;
        case ControlCommandOperation::VehicleSteerRight:
            angle -= 0.1f;
            break;
        }
    }

    float angleRadians = glm::radians<float>(angle);
    float cosTheta = glm::cos(angleRadians),
          sinTheta = glm::sin(angleRadians);

    baseTransform.worldPosition.x += speed * deltaTime * sinTheta;
    baseTransform.worldPosition.y += speed * deltaTime * cosTheta;
    baseTransform.rotation.z = angle;
}

GameObjectTransform VehicleGameObject::GetWorldTransform() const
{
    // TODO: get the transform from bullet physics model
    return baseTransform;
}

std::list<GameObjectEntity> VehicleGameObject::GetEntities() const
{
    GameObjectEntity vehicleEntity{};
    vehicleEntity.entityId = bodyEntityId;
    vehicleEntity.transform = baseTransform;
    return std::list<GameObjectEntity>({ vehicleEntity });
}

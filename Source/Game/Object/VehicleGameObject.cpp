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
    btDynamicsWorld *world = physics->GetDynamicsWorld();
    world->removeVehicle(vehicle.get());
    world->removeRigidBody(chassis.get());
}

void VehicleGameObject::Initialize()
{
    chassisMotionState = std::make_unique<btDefaultMotionState>(btTransform(btQuaternion(0, 0, 0, 1), btVector3(origin.x, origin.y, origin.z)));
    chassisShape = std::make_unique<btBoxShape>(btVector3(3, 5, 2));

    btVector3 localInertia{};
    chassisShape->calculateLocalInertia(800.0f, localInertia);

    btRigidBody::btRigidBodyConstructionInfo chassisInfo(800.0, chassisMotionState.get(), chassisShape.get(), localInertia);
    btRaycastVehicle::btVehicleTuning tuning;
    btDynamicsWorld *world = physics->GetDynamicsWorld();

    chassis = std::make_unique<btRigidBody>(chassisInfo);
    chassis->setActivationState(DISABLE_DEACTIVATION);
    world->addRigidBody(chassis.get());

    raycaster = std::make_unique<btDefaultVehicleRaycaster>(world);
    vehicle = std::make_unique<btRaycastVehicle>(tuning, chassis.get(), raycaster.get());

    // Ensure that the z-axis is the up axis
    vehicle->setCoordinateSystem(0, 2, 1);

    // TODO: hard-coded valeues for the wheels
    float wheelRadius = 0.5f;
    float suspensionRestLength = 2;
    btVector3 wheelDirectionCS0(0, 0, -1); // direction to the ground (-z-axis)
    btVector3 wheelAxleCS(1, 0, 0); // which axis does the wheel rotate about
    btVector3 connectionPointCS0(-3, 5 - wheelRadius, 0.5f); // position of the wheel relative to vehicle's origin
    wheelShape = std::make_unique<btCylinderShapeX>(btVector3(0.4f, wheelRadius, wheelRadius));

    vehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength, wheelRadius, tuning, true);
    vehicle->addWheel(connectionPointCS0 * btVector3(-1, 1, 1), wheelDirectionCS0, wheelAxleCS, suspensionRestLength, wheelRadius, tuning, true);
    vehicle->addWheel(connectionPointCS0 * btVector3(1, -1, 1), wheelDirectionCS0, wheelAxleCS, suspensionRestLength, wheelRadius, tuning, false);
    vehicle->addWheel(connectionPointCS0 * btVector3(-1, -1, 1), wheelDirectionCS0, wheelAxleCS, suspensionRestLength, wheelRadius, tuning, false);

    for (int i = 0; i < vehicle->getNumWheels(); i++)
    {
        btWheelInfo &wheel = vehicle->getWheelInfo(i);
        wheel.m_suspensionStiffness = 20.f;
        wheel.m_wheelsDampingRelaxation = 2.3f;
        wheel.m_wheelsDampingCompression = 4.4f;
        wheel.m_frictionSlip = 1000;
        wheel.m_rollInfluence = 0.1f;
    }

    world->addVehicle(vehicle.get());
}

void VehicleGameObject::Update(float deltaTime, const std::list<ControlCommand> &commands)
{
    // Do not call stepSimulation as the game object system will make the call
    // This function should simply need to apply whatever input (ex. throttle) the user gives
    // TODO: just some test code to move object
    // will be removed once this is integrated with bullet physics
    float engineForce = 1000;
    float brakeForce = 100;

    for (const ControlCommand &command : commands)
    {
        switch (command.operation)
        {
        case ControlCommandOperation::VehicleAccelerate:
            vehicle->applyEngineForce(engineForce, 2);
            vehicle->applyEngineForce(engineForce, 3);
            break;
        case ControlCommandOperation::VehicleBrake:
            vehicle->setBrake(brakeForce, 2);
            vehicle->setBrake(brakeForce, 3);
            break;
        case ControlCommandOperation::VehicleSteerLeft:
            vehicle->setSteeringValue(-1, 0);
            vehicle->setSteeringValue(-1, 1);
            break;
        case ControlCommandOperation::VehicleSteerRight:
            vehicle->setSteeringValue(1, 0);
            vehicle->setSteeringValue(1, 1);
            break;
        }
    }

    const btTransform &transform = vehicle->getChassisWorldTransform();
    const btVector3 &origin = transform.getOrigin();
    baseTransform.worldPosition = { origin.getX(), origin.getY(), origin.getZ() };

    const btTransform &wheelTransform = vehicle->getWheelTransformWS(0);
    const btQuaternion &wheelRotation = wheelTransform.getRotation();
    float wheelAngleX, wheelAngleY, wheelAngleZ;
    wheelRotation.getEulerZYX(wheelAngleZ, wheelAngleY, wheelAngleX);

    baseTransform.rotation.z = 0;
}

GameObjectTransform VehicleGameObject::GetWorldTransform() const
{
    return baseTransform;
}

std::list<GameObjectEntity> VehicleGameObject::GetEntities() const
{
    GameObjectEntity vehicleEntity{};
    vehicleEntity.entityId = bodyEntityId;
    vehicleEntity.transform = baseTransform;
    return std::list<GameObjectEntity>({ vehicleEntity });
}

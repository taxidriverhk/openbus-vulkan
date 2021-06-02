#include <btBulletDynamicsCommon.h>

#include "Game/Physics/PhysicsSystem.h"
#include "VehicleGameObject.h"

VehicleGameObject::VehicleGameObject(const VehicleGameObjectConstructionInfo &info, PhysicsSystem *physics)
    : physics(physics),
      brakeForce(0.0f),
      engineForce(0.0f),
      steering(0.0f),
      wheelRadius(0.5f)
{
    mass = info.mass;
    boundingBoxSize = info.boundingBoxSize;
    bodyEntityId = info.chassisEntityId;
    baseTransform = info.chassisStartTransform;
    centerOfMass = info.centerOfMass;

    for (uint32_t i = 0; i < info.wheels.size(); i++)
    {
        const VehicleGameObjectConstructionInfo::WheelInfo &wheelInfo = info.wheels[i];
        wheelEntityIds.push_back(wheelInfo.entityId);
        wheelTransforms.push_back(wheelInfo.transform);
        wheelRadius = wheelInfo.radius;
    }
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
    btDynamicsWorld *world = physics->GetDynamicsWorld();

    btVector3 halfExtent(boundingBoxSize.x / 2, boundingBoxSize.y / 2, boundingBoxSize.z / 2);
    chassisBoxShape = std::make_unique<btBoxShape>(halfExtent);
    
    btTransform chassisLocalTransform;
    chassisLocalTransform.setIdentity();
    chassisLocalTransform.setOrigin(btVector3(centerOfMass.x, centerOfMass.y, centerOfMass.z));
    chassisShape = std::make_unique<btCompoundShape>();
    chassisShape->addChildShape(chassisLocalTransform, chassisBoxShape.get());

    btVector3 localInertia(0, 0, 0);
    chassisShape->calculateLocalInertia(mass, localInertia);

    const glm::vec3 &startPosition = baseTransform.worldPosition;
    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(startPosition.x, startPosition.y, startPosition.z));
    chassisMotionState = std::make_unique<btDefaultMotionState>(startTransform);

    btRigidBody::btRigidBodyConstructionInfo chassisInfo(mass, chassisMotionState.get(), chassisShape.get(), localInertia);
    chassis = std::make_unique<btRigidBody>(chassisInfo);
    world->addRigidBody(chassis.get());

    btRaycastVehicle::btVehicleTuning tuning;
    raycaster = std::make_unique<btDefaultVehicleRaycaster>(world);
    vehicle = std::make_unique<btRaycastVehicle>(tuning, chassis.get(), raycaster.get());
    chassis->setActivationState(DISABLE_DEACTIVATION);
    world->addAction(vehicle.get());

    // Ensure that the z-axis is the up axis
    vehicle->setCoordinateSystem(0, 2, 1);

    float suspensionRestLength = 0.5f;
    btVector3 wheelDirectionCS0(0, 0, -1); // direction to the ground (-z-axis)
    btVector3 wheelAxleCS(1, 0, 0); // which axis does the wheel rotate about
    for (uint32_t i = 0; i < wheelTransforms.size(); i++)
    {
        // Translation relative to the origin of the chassis
        bool isFrontWheel = i < 2 ? true : false;
        const glm::vec3 &connectionPoint = wheelTransforms[i].worldPosition;
        btVector3 connectionPointCS0(connectionPoint.x, connectionPoint.y, connectionPoint.z);
        vehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength, wheelRadius, tuning, isFrontWheel);

        btWheelInfo &wheel = vehicle->getWheelInfo(i);
        wheel.m_suspensionStiffness = 25.f;
        wheel.m_wheelsDampingRelaxation = 2.3f;
        wheel.m_wheelsDampingCompression = 4.4f;
        wheel.m_frictionSlip = 1.2f;
        wheel.m_rollInfluence = 0.1f;
    }
}

void VehicleGameObject::Update(float deltaTime, const std::list<ControlCommand> &commands)
{
    // Do not call stepSimulation as the game object system will make the call
    // This function should simply need to apply whatever input (ex. throttle) the user gives
    // TODO: just some test code to move object
    // will be removed once this is integrated with bullet physics
    float maxEngineForce = 1000;
    float maxBrakeForce = 100;

    for (const ControlCommand &command : commands)
    {
        switch (command.operation)
        {
        case ControlCommandOperation::VehicleAccelerate:
            engineForce = maxEngineForce;
            brakeForce = 0;
            break;
        case ControlCommandOperation::VehicleBrake:
            engineForce = 0;
            brakeForce = maxBrakeForce;
            break;
        case ControlCommandOperation::VehicleSteerLeft:
            steering += 0.05f;
            break;
        case ControlCommandOperation::VehicleSteerRight:
            steering -= 0.05f;
            break;
        }
    }

    steering = std::clamp(steering, -MAX_STEERING_VALUE, MAX_STEERING_VALUE);
    vehicle->setSteeringValue(steering, 0);
    vehicle->setSteeringValue(steering, 1);
    vehicle->setBrake(brakeForce, 2);
    vehicle->setBrake(brakeForce, 3);
    vehicle->applyEngineForce(engineForce, 2);
    vehicle->applyEngineForce(engineForce, 3);

    const btTransform &transform = vehicle->getChassisWorldTransform();
    const btVector3 &origin = transform.getOrigin();
    const btQuaternion &chassisRotation = transform.getRotation();
    btScalar bodyYaw, bodyPitch, bodyRoll;
    chassisRotation.getEulerZYX(bodyYaw, bodyPitch, bodyRoll);

    baseTransform.worldPosition = { origin.getX(), origin.getY(), origin.getZ() };
    baseTransform.worldPosition += centerOfMass;
    baseTransform.rotation =
    {
        bodyRoll * SIMD_DEGS_PER_RAD,
        bodyPitch * SIMD_DEGS_PER_RAD,
        bodyYaw * SIMD_DEGS_PER_RAD
    };

    float speed = vehicle->getCurrentSpeedKmHour();

    for (uint32_t i = 0; i < wheelTransforms.size(); i++)
    {
        const btTransform &wheelTransform = vehicle->getWheelTransformWS(i);
        const btVector3 &wheelPosition = wheelTransform.getOrigin();
        
        const btQuaternion &wheelRotation = wheelTransform.getRotation();
        btScalar wheelYaw, wheelPitch, wheelRoll;
        wheelRotation.getEulerZYX(wheelYaw, wheelPitch, wheelRoll);

        wheelTransforms[i].worldPosition = { wheelPosition.getX(), wheelPosition.getY(), wheelPosition.getZ() };
        wheelTransforms[i].rotation =
        {
            wheelRoll * SIMD_DEGS_PER_RAD,
            wheelPitch * SIMD_DEGS_PER_RAD,
            wheelYaw * SIMD_DEGS_PER_RAD
        };
    }
}

GameObjectTransform VehicleGameObject::GetWorldTransform() const
{
    return baseTransform;
}

std::list<GameObjectEntity> VehicleGameObject::GetEntities() const
{
    std::list<GameObjectEntity> entities;

    GameObjectEntity vehicleEntity{};
    vehicleEntity.entityId = bodyEntityId;
    vehicleEntity.transform = baseTransform;
    entities.push_back(vehicleEntity);

    for (uint32_t i = 0; i < wheelEntityIds.size(); i++)
    {
        GameObjectEntity wheelEntity{};
        wheelEntity.entityId = wheelEntityIds[i];
        wheelEntity.transform = wheelTransforms[i];
        entities.push_back(wheelEntity);
    }

    return entities;
}

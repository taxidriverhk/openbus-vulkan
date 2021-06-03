#include <btBulletDynamicsCommon.h>
#include <glm/gtc/type_ptr.hpp>

#include "Game/Physics/PhysicsSystem.h"
#include "VehicleGameObject.h"

VehicleGameObject::VehicleGameObject(const VehicleGameObjectConstructionInfo &info, PhysicsSystem *physics)
    : physics(physics),
      brakeForce(0.0f),
      engineForce(0.0f),
      steering(0.0f),
      wheelRadius(0.5f)
{
    int entityIndex = 0;
    entities.resize(1 + info.wheels.size());
    wheelIndices.resize(info.wheels.size());

    mass = info.mass;
    boundingBoxSize = info.boundingBoxSize;
    centerOfMass = info.centerOfMass;

    bodyEntityIndex = entityIndex;
    entities[entityIndex++] = { info.chassisEntityId, info.chassisStartTransform };

    for (uint32_t i = 0; i < info.wheels.size(); i++)
    {
        const VehicleGameObjectConstructionInfo::WheelInfo &wheelInfo = info.wheels[i];
        wheelIndices[i] = entityIndex;
        entities[entityIndex++] = { wheelInfo.entityId, wheelInfo.transform };
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

    const glm::vec3 &startPosition = entities[bodyEntityIndex].transform.translation;
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
    for (uint32_t i = 0; i < wheelIndices.size(); i++)
    {
        // Translation relative to the origin of the chassis
        bool isFrontWheel = i < 2 ? true : false;
        const glm::vec3 &connectionPoint = entities[wheelIndices[i]].transform.translation;
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
            steering += 0.01f;
            break;
        case ControlCommandOperation::VehicleSteerRight:
            steering -= 0.01f;
            break;
        }
    }

    float speed = vehicle->getCurrentSpeedKmHour();

    steering = std::clamp(steering, -MAX_STEERING_VALUE, MAX_STEERING_VALUE);
    vehicle->setSteeringValue(steering, 0);
    vehicle->setSteeringValue(steering, 1);
    vehicle->setBrake(brakeForce, 2);
    vehicle->setBrake(brakeForce, 3);

    if (speed > MAX_SPEED_KMHR)
    {
        vehicle->applyEngineForce(0, 2);
        vehicle->applyEngineForce(0, 3);
    }
    else
    {
        vehicle->applyEngineForce(engineForce, 2);
        vehicle->applyEngineForce(engineForce, 3);
    }

    const btTransform &transform = vehicle->getChassisWorldTransform();
    const btVector3 &chassisOrigin = transform.getOrigin();
    const btQuaternion &chassisRotation = transform.getRotation();
    const btVector3 &chassisRotationAxis = chassisRotation.getAxis();

    entities[bodyEntityIndex].transform.mode = EntityTransformationMode::Quaternion;
    
    entities[bodyEntityIndex].transform.translation = { chassisOrigin.x(), chassisOrigin.y(), chassisOrigin.z() };
    entities[bodyEntityIndex].transform.translation += centerOfMass;
    
    entities[bodyEntityIndex].transform.rotationAxis = { chassisRotationAxis.x(), chassisRotationAxis.y(), chassisRotationAxis.z() };
    entities[bodyEntityIndex].transform.angle = chassisRotation.getAngle() * SIMD_DEGS_PER_RAD;

    for (uint32_t i = 0; i < wheelIndices.size(); i++)
    {
        const btTransform &wheelTransform = vehicle->getWheelTransformWS(i);
        const btVector3 &wheelOrigin = wheelTransform.getOrigin();
        const btQuaternion &wheelRotation = wheelTransform.getRotation();
        const btVector3 &wheelRotationAxis = wheelRotation.getAxis();

        entities[wheelIndices[i]].transform.mode = EntityTransformationMode::Quaternion;
        entities[wheelIndices[i]].transform.translation = { wheelOrigin.x(), wheelOrigin.y(), wheelOrigin.z() };
        entities[wheelIndices[i]].transform.rotationAxis = { wheelRotationAxis.x(), wheelRotationAxis.y(), wheelRotationAxis.z() };
        entities[wheelIndices[i]].transform.angle = wheelRotation.getAngle() * SIMD_DEGS_PER_RAD;
    }
}

EntityTransformation VehicleGameObject::GetWorldTransform() const
{
    return entities[bodyEntityIndex].transform;
}

std::vector<GameObjectEntity> VehicleGameObject::GetEntities() const
{
    return entities;
}

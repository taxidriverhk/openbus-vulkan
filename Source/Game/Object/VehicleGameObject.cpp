#include <btBulletDynamicsCommon.h>
#include <glm/gtc/type_ptr.hpp>

#include "Game/Physics/PhysicsSystem.h"
#include "VehicleGameObject.h"

VehicleGameObject::VehicleGameObject(const VehicleGameObjectConstructionInfo &info, PhysicsSystem *physics)
    : physics(physics),
      currentBrakeForce(0.0f),
      currentEngineForce(0.0f),
      currentSteeringAngle(0.0f)
{
    int entityIndex = 0;
    entities.resize(1 + info.wheels.size());
    wheelIndices.resize(info.wheels.size());
    wheels.resize(info.wheels.size());

    mass = info.mass;
    boundingBoxSize = info.boundingBoxSize;
    centerOfMass = info.centerOfMass;
    engineForce = info.engineForce;
    brakeForce = info.brakeForce;
    steeringForce = info.steeringForce;
    steeringAngle = info.steeringAngle;
    maxSpeed = info.maxSpeed;

    bodyEntityIndex = entityIndex;
    entities[entityIndex++] = { info.chassisEntityId, info.chassisStartTransform };

    for (uint32_t i = 0; i < info.wheels.size(); i++)
    {
        const VehicleGameObjectConstructionInfo::WheelInfo &wheelInfo = info.wheels[i];
        wheelIndices[i] = entityIndex;
        entities[entityIndex++] = { wheelInfo.entityId, wheelInfo.transform };
        
        wheels[i].axle = wheelInfo.axle;
        wheels[i].direction = wheelInfo.direction;
        wheels[i].radius = wheelInfo.radius;
        wheels[i].isTurnable = wheelInfo.isTurnable;
        wheels[i].hasTorque = wheelInfo.hasTorque;
        wheels[i].suspensionRestLength = wheelInfo.suspensionRestLength;
        wheels[i].suspensionStiffness = wheelInfo.suspensionStiffness;
        wheels[i].wheelsDampingRelaxation = wheelInfo.wheelsDampingRelaxation;
        wheels[i].wheelsDampingCompression = wheelInfo.wheelsDampingCompression;
        wheels[i].frictionSlip = wheelInfo.frictionSlip;
        wheels[i].rollInfluence = wheelInfo.rollInfluence;
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

    for (uint32_t i = 0; i < wheelIndices.size(); i++)
    {
        // Translation relative to the origin of the chassis
        bool isFrontWheel = i < 2 ? true : false;
        const glm::vec3 &connectionPoint = entities[wheelIndices[i]].transform.translation;
        const Wheel &wheelInfo = wheels[i];

        btVector3 connectionPointCS0(connectionPoint.x, connectionPoint.y, connectionPoint.z);
        btVector3 wheelDirectionCS0(wheelInfo.direction.x, wheelInfo.direction.y, wheelInfo.direction.z);
        btVector3 wheelAxleCS(wheelInfo.axle.x, wheelInfo.axle.y, wheelInfo.axle.z);

        btWheelInfo &wheel = vehicle->addWheel(
            connectionPointCS0,
            wheelDirectionCS0,
            wheelAxleCS,
            wheelInfo.suspensionRestLength,
            wheelInfo.radius,
            tuning,
            wheelInfo.isTurnable);
        wheel.m_suspensionStiffness = wheelInfo.suspensionStiffness;
        wheel.m_wheelsDampingRelaxation = wheelInfo.wheelsDampingRelaxation;
        wheel.m_wheelsDampingCompression = wheelInfo.wheelsDampingCompression;
        wheel.m_frictionSlip = wheelInfo.frictionSlip;
        wheel.m_rollInfluence = wheelInfo.rollInfluence;
    }
}

void VehicleGameObject::Update(float deltaTime, const std::list<ControlCommand> &commands)
{
    // Do not call stepSimulation as the game object system will make the call
    // This function should simply need to apply whatever input (ex. throttle) the user gives
    for (const ControlCommand &command : commands)
    {
        switch (command.operation)
        {
        case ControlCommandOperation::VehicleAccelerate:
            currentEngineForce = engineForce;
            currentBrakeForce = 0;
            break;
        case ControlCommandOperation::VehicleBrake:
            currentEngineForce = 0;
            currentBrakeForce = brakeForce;
            break;
        case ControlCommandOperation::VehicleSteerLeft:
            currentSteeringAngle += steeringForce;
            break;
        case ControlCommandOperation::VehicleSteerRight:
            currentSteeringAngle -= steeringForce;
            break;
        }
    }

    float speed = vehicle->getCurrentSpeedKmHour();

    currentSteeringAngle = std::clamp(currentSteeringAngle, -steeringAngle, steeringAngle);
    for (uint32_t i = 0; i < wheels.size(); i++)
    {
        if (wheels[i].hasTorque)
        {
            if (speed > maxSpeed)
            {
                vehicle->applyEngineForce(0, i);
            }
            else
            {
                vehicle->applyEngineForce(currentEngineForce, i);
            }
            vehicle->setBrake(currentBrakeForce, i);
        }
        if (wheels[i].isTurnable)
        {
            vehicle->setSteeringValue(currentSteeringAngle, i);
        }
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
        vehicle->updateWheelTransform(i, false);

        const btTransform &wheelTransform = vehicle->getWheelTransformWS(i);
        const btVector3 &wheelOrigin = wheelTransform.getOrigin();
        const btQuaternion &wheelRotation = wheelTransform.getRotation();
        const btVector3 &wheelRotationAxis = wheelRotation.getAxis();

        entities[wheelIndices[i]].transform.mode = EntityTransformationMode::Quaternion;
        entities[wheelIndices[i]].transform.translation = { wheelOrigin.x(), wheelOrigin.y(), wheelOrigin.z() };
        entities[wheelIndices[i]].transform.rotationAxis = { wheelRotationAxis.x(), wheelRotationAxis.y(), wheelRotationAxis.z() };
        entities[wheelIndices[i]].transform.angle = wheelRotation.getAngle() * SIMD_DEGS_PER_RAD;
    }

    if (physics->IsDebugDrawingEnabled())
    {
        physics->GetDynamicsWorld()->debugDrawObject(transform, chassisShape.get(), btVector3(1, 0, 0));
        vehicle->debugDraw(physics->GetDebugDrawer());
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

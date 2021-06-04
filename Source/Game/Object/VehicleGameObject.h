#pragma once

#include <memory>
#include <vector>

#include "BaseGameObject.h"

class PhysicsSystem;

class btCollisionShape;
class btCompoundShape;
class btMotionState;
class btRaycastVehicle;
class btRigidBody;
struct btVehicleRaycaster;

struct VehicleGameObjectConstructionInfo
{
    struct WheelInfo
    {
        uint32_t entityId;
        EntityTransformation transform;

        glm::vec3 axle;
        glm::vec3 direction;

        bool isTurnable;
        bool hasTorque;
        float radius;

        float suspensionRestLength;
        float suspensionStiffness;
        float wheelsDampingRelaxation;
        float wheelsDampingCompression;
        float frictionSlip;
        float rollInfluence;
    };

    uint32_t chassisEntityId;
    EntityTransformation chassisStartTransform;
    std::vector<WheelInfo> wheels;

    glm::vec3 boundingBoxSize;
    glm::vec3 centerOfMass;

    float mass;

    float maxSpeed;
    float engineForce;
    float brakeForce;
    float steeringForce;
    float steeringAngle;
};

class VehicleGameObject : public BaseGameObject
{
public:
    VehicleGameObject(const VehicleGameObjectConstructionInfo &info, PhysicsSystem *physics);
    ~VehicleGameObject();

    btRaycastVehicle *GetVehicle() const { return vehicle.get(); }

    void Destroy() override;
    void Initialize() override;
    void Update(float deltaTime, const std::list<ControlCommand> &commands) override;

    EntityTransformation GetWorldTransform() const override;
    std::vector<GameObjectEntity> GetEntities() const override;

private:
    struct Wheel
    {
        glm::vec3 axle;
        glm::vec3 direction;

        bool isTurnable;
        bool hasTorque;
        float radius;

        float suspensionRestLength;
        float suspensionStiffness;
        float wheelsDampingRelaxation;
        float wheelsDampingCompression;
        float frictionSlip;
        float rollInfluence;
    };

    PhysicsSystem *physics;

    // TODO: just some test code to show how can a game object contain multiple entities
    float maxSpeed;
    float engineForce;
    float brakeForce;
    float steeringForce;
    float steeringAngle;

    float currentSteeringAngle;
    float currentEngineForce;
    float currentBrakeForce;

    float mass;
    glm::vec3 boundingBoxSize;
    glm::vec3 centerOfMass;

    std::vector<Wheel> wheels;

    int bodyEntityIndex;
    std::vector<int> wheelIndices;
    std::vector<GameObjectEntity> entities;

    // Bullet physics related variables
    std::unique_ptr<btMotionState> chassisMotionState;
    std::unique_ptr<btCollisionShape> chassisBoxShape;
    std::unique_ptr<btCompoundShape> chassisShape;
    std::unique_ptr<btRigidBody> chassis;

    std::unique_ptr<btCollisionShape> wheelShape;

    std::unique_ptr<btVehicleRaycaster> raycaster;
    std::unique_ptr<btRaycastVehicle> vehicle;
};

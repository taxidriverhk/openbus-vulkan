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
        float radius;
        EntityTransformation transform;
    };

    uint32_t chassisEntityId;
    EntityTransformation chassisStartTransform;
    std::vector<WheelInfo> wheels;

    glm::vec3 boundingBoxSize;
    glm::vec3 centerOfMass;

    float mass;
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
    static constexpr float MAX_STEERING_VALUE = 0.25f;
    static constexpr float MAX_SPEED_KMHR = 50.0f;

    PhysicsSystem *physics;

    // TODO: just some test code to show how can a game object contain multiple entities
    float mass;
    float wheelRadius;

    float steering;
    float engineForce;
    float brakeForce;

    glm::vec3 boundingBoxSize;
    glm::vec3 centerOfMass;

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

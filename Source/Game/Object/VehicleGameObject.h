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
        GameObjectTransform transform;
    };

    uint32_t chassisEntityId;
    GameObjectTransform chassisStartTransform;
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

    GameObjectTransform GetWorldTransform() const override;
    std::list<GameObjectEntity> GetEntities() const override;

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

    uint32_t bodyEntityId;
    std::vector<uint32_t> wheelEntityIds;

    GameObjectTransform baseTransform;
    std::vector<GameObjectTransform> wheelTransforms;

    // Bullet physics related variables
    std::unique_ptr<btMotionState> chassisMotionState;
    std::unique_ptr<btCollisionShape> chassisBoxShape;
    std::unique_ptr<btCompoundShape> chassisShape;
    std::unique_ptr<btRigidBody> chassis;

    std::unique_ptr<btCollisionShape> wheelShape;

    std::unique_ptr<btVehicleRaycaster> raycaster;
    std::unique_ptr<btRaycastVehicle> vehicle;
};

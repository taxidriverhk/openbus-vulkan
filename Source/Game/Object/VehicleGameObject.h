#pragma once

#include <memory>
#include <vector>

#include "BaseGameObject.h"

class PhysicsSystem;

class btCollisionShape;
class btMotionState;
class btRaycastVehicle;
class btRigidBody;
struct btVehicleRaycaster;

class VehicleGameObject : public BaseGameObject
{
public:
    VehicleGameObject(uint32_t bodyEntityId, const GameObjectTransform &originTransform, PhysicsSystem *physics);
    ~VehicleGameObject();

    btRaycastVehicle *GetVehicle() const { return vehicle.get(); }

    void Destroy() override;
    void Initialize() override;
    void Update(float deltaTime, const std::list<ControlCommand> &commands) override;

    GameObjectTransform GetWorldTransform() const override;
    std::list<GameObjectEntity> GetEntities() const override;

private:
    // TODO: just some test code to show how can a game object contain multiple entities
    uint32_t bodyEntityId;
    std::vector<uint32_t> wheelEntityIds;

    float angle;
    float speed;
    glm::vec3 origin;
    GameObjectTransform baseTransform;

    PhysicsSystem *physics;

    std::unique_ptr<btMotionState> chassisMotionState;
    std::unique_ptr<btCollisionShape> chassisShape;
    std::unique_ptr<btRigidBody> chassis;

    std::unique_ptr<btCollisionShape> wheelShape;

    std::unique_ptr<btVehicleRaycaster> raycaster;
    std::unique_ptr<btRaycastVehicle> vehicle;
};

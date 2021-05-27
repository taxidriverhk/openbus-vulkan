#include "VehicleGameObject.h"

VehicleGameObject::VehicleGameObject(uint32_t bodyEntityId, const GameObjectTransform &originTransform)
    : baseTransform(originTransform),
      origin(originTransform.worldPosition),
      bodyEntityId(bodyEntityId),
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

}

void VehicleGameObject::Update(float deltaTime, const std::list<ControlCommand> &commands)
{
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
    return baseTransform;
}

std::list<GameObjectEntity> VehicleGameObject::GetEntities() const
{
    GameObjectEntity vehicleEntity{};
    vehicleEntity.entityId = bodyEntityId;
    vehicleEntity.transform = baseTransform;
    return std::list<GameObjectEntity>({ vehicleEntity });
}

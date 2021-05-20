#include "VehicleGameObject.h"

VehicleGameObject::VehicleGameObject(uint32_t bodyEntityId, const GameObjectTransform &originTransform)
    : baseTransform(originTransform),
      origin(originTransform.worldPosition),
      bodyEntityId(bodyEntityId),
      angle(0.0f)
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

void VehicleGameObject::Update(float deltaTime, const std::list<GameObjectCommand> &commands)
{
    // TODO: just some test code to move object
    angle += deltaTime;
    float cosTheta = glm::cos(angle),
          sinTheta = glm::sin(angle);
    float newX = origin.x + 20 * cosTheta,
          newY = origin.y + 20 * sinTheta;
    baseTransform.worldPosition.x = newX;
    baseTransform.worldPosition.y = newY;
    baseTransform.rotation.z = 20 * angle;
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

#include "VehicleGameObject.h"

VehicleGameObject::VehicleGameObject(uint32_t bodyEntityId)
    : basePosition{},
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
    float newX = 20 * glm::cos(angle),
          newY = 20 * glm::sin(angle);
    basePosition.x = newX;
    basePosition.y = newY;
}

std::list<GameObjectEntity> VehicleGameObject::GetEntities() const
{
    GameObjectEntity vehicleEntity{};
    vehicleEntity.entityId = bodyEntityId;
    vehicleEntity.worldPosition = basePosition;
    return std::list<GameObjectEntity>({ vehicleEntity });
}

#pragma once

#include <list>
#include <glm/glm.hpp>

enum class GameObjectCommand
{
    VehicleAccerlate,
    VehicleBrake,
    VehicleSteerLeft,
    VehicleSteerRight
};

struct GameObjectEntity
{
    uint32_t entityId;
    glm::vec3 worldPosition;
    glm::vec3 rotation;
};

class BaseGameObject
{
public:
    virtual void Destroy() = 0;
    virtual void Initialize() = 0;
    virtual void Update(float deltaTime, const std::list<GameObjectCommand> &commands) = 0;

    virtual std::list<GameObjectEntity> GetEntities() const = 0;
};

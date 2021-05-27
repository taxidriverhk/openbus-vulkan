#pragma once

#include <list>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Game/Control.h"

struct GameObjectTransform
{
    glm::vec3 worldPosition;
    glm::vec3 rotation;
};

struct GameObjectEntity
{
    uint32_t entityId;
    GameObjectTransform transform;
};

class BaseGameObject
{
public:
    virtual void Destroy() = 0;
    virtual void Initialize() = 0;
    virtual void Update(float deltaTime, const std::list<ControlCommand> &commands) = 0;

    virtual GameObjectTransform GetWorldTransform() const = 0;
    virtual std::list<GameObjectEntity> GetEntities() const = 0;
};

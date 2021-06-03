#pragma once

#include <list>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Engine/Entity.h"
#include "Game/Control.h"

struct GameObjectEntity
{
    uint32_t entityId;
    EntityTransformation transform;
};

class BaseGameObject
{
public:
    virtual void Destroy() = 0;
    virtual void Initialize() = 0;
    virtual void Update(float deltaTime, const std::list<ControlCommand> &commands) = 0;

    virtual EntityTransformation GetWorldTransform() const = 0;
    virtual std::vector<GameObjectEntity> GetEntities() const = 0;
};

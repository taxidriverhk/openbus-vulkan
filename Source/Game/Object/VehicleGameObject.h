#pragma once

#include <vector>

#include "BaseGameObject.h"

class VehicleGameObject : public BaseGameObject
{
public:
    VehicleGameObject(uint32_t bodyEntityId, const GameObjectTransform &originTransform);
    ~VehicleGameObject();

    void Destroy() override;
    void Initialize() override;
    void Update(float deltaTime, const std::list<GameObjectCommand> &commands) override;

    GameObjectTransform GetWorldTransform() const override;
    std::list<GameObjectEntity> GetEntities() const override;

private:
    // TODO: just some test code to show how can a game object contain multiple entities
    uint32_t bodyEntityId;
    float angle;
    float speed;
    glm::vec3 origin;
    GameObjectTransform baseTransform;
    std::vector<uint32_t> wheelEntityIds;
};

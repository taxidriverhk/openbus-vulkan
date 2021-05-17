#pragma once

#include <vector>

#include "BaseGameObject.h"

class VehicleGameObject : public BaseGameObject
{
public:
    VehicleGameObject(uint32_t bodyEntityId);
    ~VehicleGameObject();

    void Destroy() override;
    void Initialize() override;
    void Update(float deltaTime, const std::list<GameObjectCommand> &commands) override;

    std::list<GameObjectEntity> GetEntities() const override;

private:
    // TODO: just some test code to show how can a game object contain multiple entities
    uint32_t bodyEntityId;
    float angle;
    glm::vec3 basePosition;
    std::vector<uint32_t> wheelEntityIds;
};
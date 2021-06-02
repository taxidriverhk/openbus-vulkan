#pragma once

#include <string>
#include <vector>

#include "EntityConfig.h"

struct VehicleConfig
{
    std::string name;
    // TODO: the model might get changed as the vehicle game object can get more complex
    // These attributes are just used to test a basic vehicle
    std::string chassisObject;
    std::vector<EntityConfig> wheelObjects;

    Vector3DConfig boundingBoxDimensions;
    Vector3DConfig centerOfMass;

    float mass;
    float maximumWheelRadius;
};

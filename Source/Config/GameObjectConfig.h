#pragma once

#include <string>
#include <vector>

#include "EntityConfig.h"

struct WheelConfig
{
    EntityConfig object;
    
    Vector3DConfig axle;
    Vector3DConfig direction;

    bool isTurnable;
    bool hasTorque;
    float radius;

    float suspensionRestLength;
    float suspensionStiffness;
    float wheelsDampingRelaxation;
    float wheelsDampingCompression;
    float frictionSlip;
    float rollInfluence;
};

struct VehicleConfig
{
    std::string name;
    // TODO: the model might get changed as the vehicle game object can get more complex
    // These attributes are just used to test a basic vehicle
    std::string chassisObject;
    std::vector<WheelConfig> wheels;

    Vector3DConfig boundingBoxDimensions;
    Vector3DConfig centerOfMass;

    float mass;
    
    float maxSpeed;
    float engineForce;
    float brakeForce;
    float steeringForce;
    float steeringAngle;
};

#include "Common/JsonParser.h"
#include "ConfigReader.h"

#include "EntityConfig.h"
#include "GameObjectConfig.h"
#include "MapConfig.h"
#include "ObjectConfig.h"
#include "SettingsConfig.h"
#include "TerrainConfig.h"

bool ConfigReader::registered = false;

void ConfigReader::RegisterConfigMapping()
{
    // Static Object
    JsonParser::RegisterMapper(&Vector2DConfig::x, "x");
    JsonParser::RegisterMapper(&Vector2DConfig::y, "y");
    JsonParser::RegisterMapper(&Vector3DConfig::x, "x");
    JsonParser::RegisterMapper(&Vector3DConfig::y, "y");
    JsonParser::RegisterMapper(&Vector3DConfig::z, "z");

    JsonParser::RegisterMapper(&StaticObjectConfig::name, "name");
    JsonParser::RegisterMapper(&StaticObjectConfig::mesh, "mesh");
    JsonParser::RegisterMapper(&StaticObjectConfig::diffuseMaterial, "diffuseMaterial");

    JsonParser::RegisterMapper(&EntityConfig::id, "id");
    JsonParser::RegisterMapper(&EntityConfig::position, "position");
    JsonParser::RegisterMapper(&EntityConfig::rotation, "rotation");
    JsonParser::RegisterMapper(&EntityConfig::object, "object");

    JsonParser::RegisterMapper(&TerrainConfig::heightMap, "heightMap");
    JsonParser::RegisterMapper(&TerrainConfig::baseTexture, "baseTexture");
    JsonParser::RegisterMapper(&TerrainConfig::textureSize, "textureSize");

    JsonParser::RegisterMapper(&MapBlockInfoConfig::offset, "offset");
    JsonParser::RegisterMapper(&MapBlockInfoConfig::terrain, "terrain");
    JsonParser::RegisterMapper(&MapBlockInfoConfig::entities, "entities");

    JsonParser::RegisterMapper(&MapBlockFileConfig::position, "position");
    JsonParser::RegisterMapper(&MapBlockFileConfig::file, "file");

    JsonParser::RegisterMapper(&MapInfoConfig::name, "name");
    JsonParser::RegisterMapper(&MapInfoConfig::image, "image");
    JsonParser::RegisterMapper(&MapInfoConfig::skyBoxImage, "skyBoxImage");
    JsonParser::RegisterMapper(&MapInfoConfig::blocks, "blocks");

    // Game Object
    JsonParser::RegisterMapper(&WheelConfig::object, "object");
    JsonParser::RegisterMapper(&WheelConfig::axle, "axle");
    JsonParser::RegisterMapper(&WheelConfig::direction, "direction");
    JsonParser::RegisterMapper(&WheelConfig::isTurnable, "isTurnable");
    JsonParser::RegisterMapper(&WheelConfig::hasTorque, "hasTorque");
    JsonParser::RegisterMapper(&WheelConfig::radius, "radius");
    JsonParser::RegisterMapper(&WheelConfig::suspensionRestLength, "suspensionRestLength");
    JsonParser::RegisterMapper(&WheelConfig::suspensionStiffness, "suspensionStiffness");
    JsonParser::RegisterMapper(&WheelConfig::wheelsDampingRelaxation, "wheelsDampingRelaxation");
    JsonParser::RegisterMapper(&WheelConfig::wheelsDampingCompression, "wheelsDampingCompression");
    JsonParser::RegisterMapper(&WheelConfig::frictionSlip, "frictionSlip");
    JsonParser::RegisterMapper(&WheelConfig::rollInfluence, "rollInfluence");

    JsonParser::RegisterMapper(&VehicleConfig::name, "name");
    JsonParser::RegisterMapper(&VehicleConfig::mass, "mass");
    JsonParser::RegisterMapper(&VehicleConfig::chassisObject, "chassisObject");
    JsonParser::RegisterMapper(&VehicleConfig::wheels, "wheels");
    JsonParser::RegisterMapper(&VehicleConfig::boundingBoxDimensions, "boundingBoxDimensions");
    JsonParser::RegisterMapper(&VehicleConfig::centerOfMass, "centerOfMass");
    JsonParser::RegisterMapper(&VehicleConfig::engineForce, "engineForce");
    JsonParser::RegisterMapper(&VehicleConfig::brakeForce, "brakeForce");
    JsonParser::RegisterMapper(&VehicleConfig::steeringForce, "steeringForce");
    JsonParser::RegisterMapper(&VehicleConfig::steeringAngle, "steeringAngle");
    JsonParser::RegisterMapper(&VehicleConfig::maxSpeed, "maxSpeed");

    // Settings
    JsonParser::RegisterMapper(&GeneralSettings::showDebugInfo, "showDebugInfo");
    JsonParser::RegisterMapper(&GeneralSettings::debugInfoUpdateFrameRate, "debugInfoUpdateFrameRate");

    JsonParser::RegisterMapper(&GraphicsSettings::enableFog, "enableFog");
    JsonParser::RegisterMapper(&GraphicsSettings::targetFrameRate, "targetFrameRate");
    JsonParser::RegisterMapper(&GraphicsSettings::maxViewableDistance, "maxViewableDistance");
    JsonParser::RegisterMapper(&GraphicsSettings::screenWidth, "screenWidth");
    JsonParser::RegisterMapper(&GraphicsSettings::screenHeight, "screenHeight");

    JsonParser::RegisterMapper(&ControlSettings::cameraMovementSpeed, "cameraMovementSpeed");
    JsonParser::RegisterMapper(&ControlSettings::cameraAngleChangeSensitivity, "cameraAngleChangeSensitivity");
    JsonParser::RegisterMapper(&ControlSettings::cameraZoomSensitivity, "cameraZoomSensitivity");

    JsonParser::RegisterMapper(&MapLoadSettings::maxAdjacentBlocks, "maxAdjacentBlocks");

    JsonParser::RegisterMapper(&GameSettings::generalSettings, "generalSettings");
    JsonParser::RegisterMapper(&GameSettings::controlSettings, "controlSettings");
    JsonParser::RegisterMapper(&GameSettings::graphicsSettings, "graphicsSettings");
    JsonParser::RegisterMapper(&GameSettings::mapLoadSettings, "mapLoadSettings");
}

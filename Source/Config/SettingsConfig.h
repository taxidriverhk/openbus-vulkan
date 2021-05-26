#pragma once

#include <string>
#include <unordered_map>

struct KeyBindings
{
    std::unordered_map<std::string, std::string> keyControlBindings;
};

struct ControlSettings
{
    float cameraMovementSpeed;
    float cameraAngleChangeSensitivity;
    float cameraZoomSensitivity;
};

struct GraphicsSettings
{
    bool enableFog;
    int targetFrameRate;
    int maxViewableDistance;
    int screenWidth;
    int screenHeight;
};

struct MapLoadSettings
{
    int maxAdjacentBlocks;
};

struct GameSettings
{
    ControlSettings controlSettings;
    GraphicsSettings graphicsSettings;
    MapLoadSettings mapLoadSettings;
};

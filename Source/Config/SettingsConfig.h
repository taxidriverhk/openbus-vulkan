#pragma once

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
    GraphicsSettings graphicsSettings;
    MapLoadSettings mapLoadSettings;
};

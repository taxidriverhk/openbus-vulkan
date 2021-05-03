#pragma once

struct GraphicsSettings
{
    int targetFrameRate;
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

#pragma once

struct GraphicsSettings
{
    int targetFrameRate;
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

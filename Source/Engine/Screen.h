#pragma once

#include <string>

#include "SDL.h"
#include "SDL_vulkan.h"

class Screen
{

public:
    Screen(const int &width, const int &height, const std::string &title);
    ~Screen();

    SDL_Window * GetWindow() const { return screen; }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    bool IsResized() const { return resized; }
    void ResetResizeState() { resized = false; }

    void Close();
    void Create();
    void Refresh();
    bool ShouldClose();

private:
    int width;
    int height;
    std::string title;

    bool resized;

    SDL_Event lastEvent;
    SDL_Window *screen;
};

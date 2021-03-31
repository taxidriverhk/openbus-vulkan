#pragma once

#define GLFW_INCLUDE_VULKAN
#include "SDL.h"
#include "SDL_vulkan.h"
#include <GLFW/glfw3.h>
#include <string>

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

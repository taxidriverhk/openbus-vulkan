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

    SDL_Window *GetWindow() const;

    void Close();
    void Create();
    void Refresh();
    bool ShouldClose();

private:
    int width;
    int height;
    std::string title;

    SDL_Event lastEvent;
    SDL_Window *screen;
};

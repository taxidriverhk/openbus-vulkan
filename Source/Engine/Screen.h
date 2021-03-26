#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

class Screen
{

public:
    Screen(int width, int height, std::string title);
    ~Screen();

    void Close();
    void OpenScreen();
    void Refresh();
    bool ShouldClose();

private:
    int width;
    int height;
    std::string title;
    GLFWwindow *screen;
};

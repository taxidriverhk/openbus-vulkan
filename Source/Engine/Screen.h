#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

class Screen
{

public:
    Screen(const int &width, const int &height, const std::string &title);
    ~Screen();

    GLFWwindow *GetWindow() const;

    void Close();
    void Create();
    void Refresh();
    bool ShouldClose();

private:
    int width;
    int height;
    std::string title;
    GLFWwindow *screen;
};

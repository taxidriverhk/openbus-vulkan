#include "Screen.h"

Screen::Screen(int width, int height, std::string title)
      : screen(nullptr),
        width(width),
        height(height),
        title(title)
{
}

Screen::~Screen()
{
}

void Screen::Close()
{
    glfwDestroyWindow(screen);
    glfwTerminate();
}

void Screen::OpenScreen()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    screen = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}

void Screen::Refresh()
{
    glfwPollEvents();
}

bool Screen::ShouldClose()
{
    return glfwWindowShouldClose(screen);
}

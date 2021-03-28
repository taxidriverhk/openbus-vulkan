#include "Screen.h"

Screen::Screen(const int &width, const int &height, const std::string &title)
      : screen(nullptr),
        width(width),
        height(height),
        title(title)
{
}

Screen::~Screen()
{
}

GLFWwindow *Screen::GetWindow() const
{
    return screen;
}

void Screen::Close()
{
    glfwDestroyWindow(screen);
    glfwTerminate();
}

void Screen::Create()
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

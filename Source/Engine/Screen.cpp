#include "Screen.h"

Screen::Screen(const int &width, const int &height, const std::string &title)
      : screen(nullptr),
        lastEvent(),
        width(width),
        height(height),
        title(title)
{
}

Screen::~Screen()
{
}

SDL_Window *Screen::GetWindow() const
{
    return screen;
}

void Screen::Close()
{
    SDL_DestroyWindow(screen);
    SDL_Quit();
}

void Screen::Create()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    screen = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);
}

void Screen::Refresh()
{
    SDL_PollEvent(&lastEvent);
}

bool Screen::ShouldClose()
{
    return lastEvent.type == SDL_QUIT;
}

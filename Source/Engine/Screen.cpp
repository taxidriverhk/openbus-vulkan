#include "Screen.h"

Screen::Screen(const int &width, const int &height, const std::string &title)
      : screen(nullptr),
        resized(false),
        lastEvent(),
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
    lastEvent.type = SDL_FIRSTEVENT;
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
        SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
}

void Screen::Refresh()
{
    SDL_PollEvent(&lastEvent);
    switch (lastEvent.type)
    {
    case SDL_WINDOWEVENT:
        SDL_WindowEvent windowEvent = lastEvent.window;
        if (windowEvent.event == SDL_WINDOWEVENT_SIZE_CHANGED
            || windowEvent.event == SDL_WINDOWEVENT_MINIMIZED
            || windowEvent.event == SDL_WINDOWEVENT_RESIZED)
        {
            int newWidth, newHeight;
            SDL_GetWindowSize(screen, &newWidth, &newHeight);

            width = newWidth;
            height = newHeight;
            resized = true;
        }
        break;
    case SDL_KEYDOWN:
        // TODO: use a new controller class to queue the inputs
        break;
    }
}

bool Screen::ShouldClose()
{
    return lastEvent.type == SDL_QUIT;
}

#include "Screen.h"

Screen::Screen(const int &width, const int &height, const std::string &title)
      : resized(false),
        width(width),
        height(height),
        title(title)
{
    lastEvent.type = sf::Event::Count;
}

Screen::~Screen()
{
}

void Screen::Close()
{
    window->close();
}

void Screen::Create()
{
    window = std::make_unique<sf::WindowBase>(sf::VideoMode(width, height), title.c_str(), sf::Style::Resize);
    window->setVisible(false);
}

void Screen::Refresh()
{
    while (window->pollEvent(lastEvent))
    {
        if (lastEvent.type == sf::Event::Resized)
        {
            sf::Vector2u newSize = window->getSize();
            width = newSize.x;
            height = newSize.y;
            resized = true;
        }
    }
}

bool Screen::ShouldClose()
{
    return lastEvent.type == sf::Event::Closed;
}

void Screen::Show()
{
    window->setVisible(true);
}

void Screen::Wait()
{
    sf::Event eventToWait;
    window->waitEvent(eventToWait);
}

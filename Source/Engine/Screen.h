#pragma once

#include <string>

#include <SFML/Window.hpp>

class Screen
{

public:
    Screen(const int &width, const int &height, const std::string &title);
    ~Screen();

    sf::WindowBase *GetWindow() { return window.get(); }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    bool IsResized() const { return resized; }
    void ResetResizeState() { resized = false; }

    void Close();
    void Create();
    void Refresh();
    bool ShouldClose();
    void Show();
    void Wait();

private:
    int width;
    int height;
    std::string title;

    bool resized;

    sf::Event lastEvent;
    std::unique_ptr<sf::WindowBase> window;
};

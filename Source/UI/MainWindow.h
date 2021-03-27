#pragma once

#include "Game/Game.h"
#include "Common/Util.h"

#include <QApplication>
#include <QtWidgets>

class MainWindow : public QWidget
{

public:
    MainWindow();
    ~MainWindow();

    void Open();

private:
    static constexpr int WINDOW_WIDTH = 1024;
    static constexpr int WINDOW_HEIGHT = 768;

    void ShutdownButtonClicked();
    void StartButtonClicked();

    std::unique_ptr<Game> game;
    std::unique_ptr<QWidget> window;
    std::unique_ptr<QPushButton> shutdownButton;
    std::unique_ptr<QPushButton> startButton;
};

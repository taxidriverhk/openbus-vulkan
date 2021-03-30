#pragma once

#include "Game/Game.h"
#include "Common/Util.h"

#include <QMainWindow>
#include <QtWidgets>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    void Open();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    static constexpr int WINDOW_WIDTH = 1024;
    static constexpr int WINDOW_HEIGHT = 768;

    void ShutdownButtonClicked();
    void StartButtonClicked();

    std::unique_ptr<Game> game;
    std::unique_ptr<QWidget> mainLayout;
    std::unique_ptr<QPushButton> shutdownButton;
    std::unique_ptr<QPushButton> startButton;
};

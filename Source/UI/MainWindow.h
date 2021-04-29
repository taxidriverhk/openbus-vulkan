#pragma once

#include <thread>

#include <QMainWindow>
#include <QtWidgets>

#include "Game/Game.h"
#include "Common/Util.h"

struct GameSessionConfig;
class MapList;
class LogViewer;

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

    void EndGame();

    void EnableStartButton();
    void ExitButtonClicked();
    void ShutdownButtonClicked();
    void StartButtonClicked();

    GameSessionConfig startConfig;
    std::unique_ptr<std::thread> gameThread;
    std::unique_ptr<Game> game;

    std::unique_ptr<QWidget> mainLayout;
    std::unique_ptr<QGridLayout> gridLayout;

    QMenu *gameMenu;
    std::unique_ptr<QAction> startAction;
    std::unique_ptr<QAction> shutdownAction;
    std::unique_ptr<QAction> exitAction;

    std::unique_ptr<QDockWidget> gameScreen;
    std::unique_ptr<LogViewer> logViewer;
    std::unique_ptr<MapList> mapList;
};

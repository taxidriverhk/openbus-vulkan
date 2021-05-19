#pragma once

#include <QMainWindow>
#include <QtWidgets>

#include "Game/Game.h"
#include "Common/Util.h"

struct GameSessionConfig;
class HandledThread;

class GameScreen;
class MapList;
class LogViewer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    void Open();

Q_SIGNALS:
    void GameThreadError();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    static constexpr int WINDOW_WIDTH = 1024;
    static constexpr int WINDOW_HEIGHT = 768;

    void EndGame();

    void AddVehicleButtonClicked();
    void ResetButtonState();
    void ExitButtonClicked();
    void ShutdownButtonClicked();
    void StartButtonClicked();

    GameSessionConfig startConfig;
    std::unique_ptr<HandledThread> gameThread;
    std::unique_ptr<Game> game;

    std::unique_ptr<QWidget> mainLayout;
    std::unique_ptr<QGridLayout> gridLayout;

    QMenu *gameMenu;
    std::unique_ptr<QAction> exitAction;

    std::unique_ptr<GameScreen> gameScreen;
    std::unique_ptr<LogViewer> logViewer;
    std::unique_ptr<MapList> mapList;
};

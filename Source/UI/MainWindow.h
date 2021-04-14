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

    void ExitButtonClicked();
    void ShutdownButtonClicked();
    void StartButtonClicked();

    std::unique_ptr<Game> game;

    std::unique_ptr<QWidget> mainLayout;

    QMenu *gameMenu;
    std::unique_ptr<QAction> startAction;
    std::unique_ptr<QAction> shutdownAction;
    std::unique_ptr<QAction> exitAction;
};

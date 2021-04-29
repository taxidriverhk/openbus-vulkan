#include "Common/HandledThread.h"
#include "Config/GameConfig.h"
#include "MapList.h"
#include "MessageDialog.h"
#include "LogViewer.h"
#include "MainWindow.h"

MainWindow::MainWindow()
{
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    setWindowTitle(Util::FormatWindowTitle("Main Window").c_str());

    mainLayout = std::make_unique<QWidget>();
    gridLayout = std::make_unique<QGridLayout>(this);

    mainLayout->setLayout(gridLayout.get());

    startAction = std::make_unique<QAction>("Start", this);
    startAction->setEnabled(false);
    shutdownAction = std::make_unique<QAction>("Shutdown", this);
    shutdownAction->setEnabled(false);
    exitAction = std::make_unique<QAction>("Exit", this);

    gameMenu = menuBar()->addMenu("Game");
    gameMenu->addAction(startAction.get());
    gameMenu->addAction(shutdownAction.get());
    gameMenu->addSeparator();
    gameMenu->addAction(exitAction.get());

    setCentralWidget(mainLayout.get());

    gameScreen = std::make_unique<QDockWidget>();
    gameScreen->setFeatures(QDockWidget::DockWidgetFeature::NoDockWidgetFeatures);
    gridLayout->setRowStretch(0, 15);
    gridLayout->addWidget(gameScreen.get(), 0, 0);

    mapList = std::make_unique<MapList>();
    mapList->setFeatures(QDockWidget::DockWidgetFeature::NoDockWidgetFeatures);
    gridLayout->setRowStretch(1, 50);
    gridLayout->addWidget(mapList.get(), 1, 0);

    logViewer = std::make_unique<LogViewer>();
    logViewer->setFeatures(QDockWidget::DockWidgetFeature::NoDockWidgetFeatures);
    gridLayout->setRowStretch(2, 35);
    gridLayout->addWidget(logViewer.get(), 2, 0);

    connect(mapList.get(), &MapList::MapListItemSelected, this, &MainWindow::EnableStartButton);
    connect(exitAction.get(), &QAction::triggered, this, &MainWindow::ExitButtonClicked);
    connect(startAction.get(), &QAction::triggered, this, &MainWindow::StartButtonClicked);
    connect(shutdownAction.get(), &QAction::triggered, this, &MainWindow::ShutdownButtonClicked);
    connect(this, &MainWindow::GameThreadError, this, &MainWindow::EndGame);
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    EndGame();
    event->accept();
}

void MainWindow::Open()
{
    show();
}

void MainWindow::EndGame()
{
    if (game != nullptr)
    {
        game->SetShouldEndGame(true);
        gameThread->Join();
    }

    EnableStartButton();
}

void MainWindow::EnableStartButton()
{
    startAction->setDisabled(false);
    shutdownAction->setDisabled(true);
}

void MainWindow::ExitButtonClicked()
{
    close();
}

void MainWindow::ShutdownButtonClicked()
{
    EndGame();
}

void MainWindow::StartButtonClicked()
{
    startAction->setDisabled(true);
    shutdownAction->setDisabled(false);
    
    std::string mapPath;
    if (mapList->GetSelectedMapFile(mapPath))
    {
        startConfig.mapConfigPath = mapPath;
        gameThread = std::make_unique<HandledThread>([&]()
            {
                game = std::make_unique<Game>();
                game->Start(startConfig);
            },
            [&]()
            {
                GameThreadError();
            });
    }
    else
    {
        MessageDialog mapNotFoundDialog;
        mapNotFoundDialog.ShowMessage(MessageDialogType::Alert, "Unable to find the map file");
        EndGame();
    }
}
